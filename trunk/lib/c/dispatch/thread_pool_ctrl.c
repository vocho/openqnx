/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




#include <errno.h>
#include <atomic.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/dispatch.h>
#include "dispatch.h"

void *_thread_pool_context_thread(void *data);

// we normalise some of the values here
// if hi > max then set hi and max to the smaller of the two
// if lo > hi, then set lo and hi to the smaller of the two
void _thread_pool_attr_normalise(thread_pool_attr_t *tpattr)
{
	if (tpattr == NULL) 
		return;
	// normalise simple absurd values
	// we check hi first, because, if hi changes, lo might also be
	// affected below.
	// be conservative, if hi > max, then set both to the lower value
	if (tpattr->hi_water > tpattr->maximum) {
		tpattr->hi_water = tpattr->maximum;
	}
	// if lo > high, then set both to the lower value
	/// also guarantee that nothing is less than zero
	if (tpattr->lo_water > tpattr->hi_water) {
		tpattr->lo_water = tpattr->hi_water;
	}
	return;
}

int thread_pool_destroy(thread_pool_t *pool) {
	int		num;
	void	*ctp;
	struct _pool_properties *props;

	props = (struct _pool_properties *)pool->props;
	/*
	 Indicate our intention to exit. No further threads
	 should be created when this flag is set and the
	 hi_water and maximum are set to 0.
	*/
	_mutex_lock(&(props->inline_lock));
	pool->flags |= (POOL_FLAG_EXITING | POOL_FLAG_CHANGING);
	pool->pool_attr.maximum = 0;
	pool->pool_attr.hi_water = 0;
	pool->pool_attr.lo_water = 0;
	_mutex_unlock(&(props->inline_lock));

	/*
	 Call the unblock function once for every thread that 
	 is created.  Note that we might over call here since 
	 someone might die between the created and num call.
	 There should never be undercall though since the
	 maximum and exiting flags should catch that.
	*/
	if ((ctp = pool->pool_attr.context_alloc(pool->pool_attr.handle))) {
		num = pool->created;
		while(num--) {
			if(pool->pool_attr.unblock_func) {
				pool->pool_attr.unblock_func(ctp);
			}
		}
		pool->pool_attr.context_free(ctp);
	}

	/*
	 Wait for everyone to die.  
	*/
	_mutex_lock(&(props->inline_lock));
	while(pool->created > 0) {
		pthread_cond_wait(&(props->pool_cond), &(props->inline_lock));
	}
	_mutex_unlock(&(props->inline_lock));

    /* destroy pool cond */
    pthread_mutex_destroy(&(props->pool_cond));
    /* destroy pool mutex */
    pthread_mutex_destroy(&(props->inline_lock));
	/* free properties structure */
	free(props);
	/* Free and clear */
	free(pool);
	return(0);
}


/* We have to be a bit careful about how we change these values.
The thread_pool will sort itself out in terms of generating the
right number of threads, or slaying them off (except in the case
where we are over the maximum and should "shutdown" the thread
pool to maximum threads).  We never want to be in a case where
there are no threads in the system (that is a shutdown scenario).
- Change maximum first (limit this to > 0?)
 -> This sets the upper limit on the system in terms of how many
	threads are created/destroyed. Because it is a limiting factor
	for the system, it should be set first.
- Change the lowater
 -> The lowwater can temporarily be higher than the highwater, this
    means that any threads that get created will automatically be
	destroyed when they try and run.  
- Change the hiwater
 -> If the highwater is lower than the lowater there is a possiblity
    of all threads exiting, thus leaving you with an empty thread
	pool.  The result is to change it after the lowater.
- Change the increment
 -> This value is always bounded, change it at will.

It is up to the user to make these functions thread safe.
We should probably do that ourselves though with a reference
count or something in the thread pool.
*/
int thread_pool_control(thread_pool_t *pool, thread_pool_attr_t *attr, 
						uint16_t lower, uint16_t upper, unsigned flags) {
	int done;
	struct _pool_properties *props;

	props = (struct _pool_properties *)pool->props;

	if(lower > upper) {
		errno = EINVAL;
		return -1;
	}

	_mutex_lock(&(props->inline_lock)); // we lock now
	props->control_threads++;
	// only one thread can be actively manipulating the 
	// limits. All others wait for that one to complete
	while (pool->flags & POOL_FLAG_CONTROL) {
		// if there is already someone waiting for a change
		// wake them up
		if (props->control_threads > 1) {
			pthread_cond_signal(&(props->pool_cond));
		}
		pthread_cond_wait(&(props->pool_cond), &(props->inline_lock));
	}
	props->control_threads--;
	pool->flags |= POOL_FLAG_CONTROL;

	if(flags & THREAD_POOL_CONTROL_MAXIMUM) {
		pool->pool_attr.maximum = attr->maximum;
	}
	if(flags & THREAD_POOL_CONTROL_LOWATER) {
		pool->pool_attr.lo_water = attr->lo_water;
	}
	if(flags & THREAD_POOL_CONTROL_HIWATER) {
		pool->pool_attr.hi_water = attr->hi_water;
	}
	if(flags & THREAD_POOL_CONTROL_INCREMENT) {
		pool->pool_attr.increment = attr->increment;
	}

	// normalise new pool attr values to be reasonably sane
	_thread_pool_attr_normalise(&(pool->pool_attr));
	/* If we have more threads created than the MAXIMUM value, or
	   we have more threads waiting than the HI_WATER value, then we 
	   should slay them off to bring the number down.  
	   If we have less threads waiting than the LO_WATER then we 
	   create one more thread which will in turn spawn others.
	*/
	done = 0;
	// we are locked when we enter here
	while(!done) {
		if((pool->waiting + props->newthreads) < pool->pool_attr.lo_water) {
			pool->created++;
			props->newthreads++;
			_mutex_unlock(&(props->inline_lock));
			if(pthread_create(0, pool->pool_attr.attr, 
                        _thread_pool_context_thread, pool) != EOK) {
				_mutex_lock(&(props->inline_lock));
				pool->created--;
				props->newthreads--;;
				pool->flags &= ~POOL_FLAG_CONTROL;
				pthread_cond_signal(&(props->pool_cond));
				_mutex_unlock(&(props->inline_lock));
				return -2;
			}
		} else if(((pool->created-props->control_threads) > pool->pool_attr.maximum) ||
				  (pool->waiting > pool->pool_attr.hi_water)) {
			int		count;
			void	*ctp;
			if(!(ctp = pool->pool_attr.context_alloc(pool->pool_attr.handle))) {
				pool->flags &= ~POOL_FLAG_CONTROL;
				pthread_cond_signal(&(props->pool_cond));
				_mutex_unlock(&(props->inline_lock));
				return -3;
			}
			/* NOTE: We run the risk of unblocking more threads than are
			   required since some threads may just have died so our created
			   value is stale.  This function may over unblock, but will loop
			   to guarantee that there will always be lo_water threads. */
			count = __max((int)((pool->created-props->control_threads) - pool->pool_attr.maximum), 0);
			count = __max((int)(pool->waiting - pool->pool_attr.hi_water), count);
			_mutex_unlock(&(props->inline_lock));
			
			while(count-- > 0) {
				if(pool->pool_attr.unblock_func) {
					pool->pool_attr.unblock_func(ctp);
				}
			}
			pool->pool_attr.context_free(ctp);
	} else {
			pool->flags &= ~POOL_FLAG_CONTROL;
			pthread_cond_signal(&(props->pool_cond));
			_mutex_unlock(&(props->inline_lock));
			done = 1;
			break;
		}

		/* Wait until the number of threads created falls between
		   the upper and lower limits provided.  */ 
		if(!(flags & THREAD_POOL_CONTROL_NONBLOCK)) {
			_mutex_lock(&(props->inline_lock));
			pool->flags |= POOL_FLAG_CHANGING;
			props->control_threads++;
			while(!((lower <= ((pool->created-props->control_threads)+1)) && 
				(((pool->created-props->control_threads)+1) <= upper))) {
				pthread_cond_wait(&(props->pool_cond), &(props->inline_lock));
			}
			props->control_threads--;
			pool->flags &= ~POOL_FLAG_CHANGING;
			flags |= THREAD_POOL_CONTROL_NONBLOCK;
			_mutex_unlock(&(props->inline_lock));
		} 
		(void)sched_yield();
		if (!done) // if we need to loop again, we lock here
			_mutex_lock(&(props->inline_lock));
	}
	return 0;
}

/* Change the limits of one or more of the number control fields for the thread pool. */
int thread_pool_limits(thread_pool_t *pool, int lowater, int hiwater, 
                        int maximum, int increment, unsigned flags) {
	thread_pool_attr_t tpattr;
	uint16_t		   upper, lower;

	memset(&tpattr, 0, sizeof(tpattr));
	flags &= (THREAD_POOL_CONTROL_NONBLOCK);
	upper = USHRT_MAX;
	lower = 0;

	//TODO: Range check on integer->uint16_t conversion and ERANGE on error

	if(hiwater >= 0) {
		tpattr.hi_water = (uint16_t)hiwater;
		flags |= THREAD_POOL_CONTROL_HIWATER;
	}
	if(lowater >= 0) {
		lower = tpattr.lo_water = __max((uint16_t)lowater, 0);
		flags |= THREAD_POOL_CONTROL_LOWATER;
	}	
	if(increment >= 0) {
		tpattr.increment = __max((uint16_t)increment, 0);
		flags |= THREAD_POOL_CONTROL_INCREMENT;
	}	
	if(maximum >= 0) {
		upper = tpattr.maximum = __max((uint16_t)maximum, 0);
		flags |= THREAD_POOL_CONTROL_MAXIMUM;
	}	

	return thread_pool_control(pool, &tpattr, lower, upper, flags);
}

__SRCVERSION("thread_pool_ctrl.c $Rev: 167279 $");
