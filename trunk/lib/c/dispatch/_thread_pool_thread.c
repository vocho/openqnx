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




#include <sys/dispatch.h>
#include <errno.h>
#include <atomic.h>
#include <stdlib.h>
#include "dispatch.h"

#define PCP_FLAG_WAITING 0x0001
struct _pool_context {
	thread_pool_t		*pool;
	void *ctp;
	unsigned			flags;
};

void *_thread_pool_thread(thread_pool_t *pool, void *ctp);
void *_thread_pool_reserve_thread(void *data);
void *_thread_pool_context_thread(void *data);

static void _thread_cleanup(void *data) {
	struct _pool_context *pcp = data;
	thread_pool_t        *pool = pcp->pool;
	struct _pool_properties *props;

	props = (struct _pool_properties *)pool->props;
	_mutex_lock(&(props->inline_lock));
	pool->waiting--;
	pool->created--;
	//Optimize on exit by only signalling once
	if(pool->flags & POOL_FLAG_CHANGING) {
		pthread_cond_signal(&(props->pool_cond));
	}
	_mutex_unlock(&(props->inline_lock));

	pool->pool_attr.context_free(pcp->ctp);

}

static void _thread_pool_update(thread_pool_t *pool, int w_adj, int n_adj) {
	int	     add=0;
	struct _pool_properties *props;
	int realwaiting;

	props = (struct _pool_properties *)pool->props;
	_mutex_lock(&(props->inline_lock));
	pool->waiting += w_adj;
	props->newthreads += n_adj;
	realwaiting = (pool->waiting + props->newthreads);
	if ((realwaiting < (pool->pool_attr.lo_water + props->reserved_threads) ) && 
			(pool->created < (pool->pool_attr.maximum + props->reserved_threads))) {
		if (pool->created < (pool->pool_attr.lo_water + props->reserved_threads))
			add = (pool->pool_attr.lo_water + props->reserved_threads) - pool->created;
		else {
			// routine increment
			add = pool->pool_attr.increment;
			// but never allow more than hi_water waiting
			// this takes care of very strange increment values
			if ((realwaiting + add) > (pool->pool_attr.hi_water + props->reserved_threads))
				add = (pool->pool_attr.hi_water + props->reserved_threads) - realwaiting;
		}
		// but not more than maximum
		if ((pool->created + add) > (pool->pool_attr.maximum + props->reserved_threads))
			add = (pool->pool_attr.maximum + props->reserved_threads) - pool->created;
	}
	if(add <= 0) {
		_mutex_unlock(&(props->inline_lock));
		return;
	}
	pool->created += add;
	props->newthreads += add;
	_mutex_unlock(&(props->inline_lock));
	// all is fine, we go on to create the required
	// number of threads.
	while (add--) {
		if(pthread_create(0, pool->pool_attr.attr, _thread_pool_context_thread, 
       pool) != EOK) {
			_mutex_lock(&(props->inline_lock));
			pool->created--;
			props->newthreads--;
			_mutex_unlock(&(props->inline_lock));
		}
	}
	// Notify if we are waiting for a change in state
	if(pool->flags & POOL_FLAG_CHANGING) {
		_mutex_lock(&(props->inline_lock));
		pthread_cond_signal(&(props->pool_cond));
		_mutex_unlock(&(props->inline_lock));
	}
}

// this is called by proc for now
// we might make this is a public interface
// via thread_pool_reserve some time soon
int _thread_pool_reserve(thread_pool_t *pool, int wait_count)
{
	int		  ret=0;
	struct _pool_properties *props;
	pthread_t tid;
	void *ctp;
	struct _pool_context *pcp = NULL;
	props = (struct _pool_properties *)pool->props;

	_mutex_lock(&(props->inline_lock));
	/* For now we don't have to do any adjustment on dropping */
  	if (wait_count < 0) {
    		props->reserved_threads--;
    		_mutex_unlock(&(props->inline_lock));
    	return 0;
  	}
	props->reserved_threads++;
  	if (pool->waiting > (wait_count + props->reserved_threads)) {
    		_mutex_unlock(&(props->inline_lock));
    	return 0;
  	}

	/*
	This thread is created outside of the realm of the maximum/hi_water
	limits since it is essentially a "I demand a thread" call.
	This unfortunately is not a guarantee of service for this thread,
	it may still be picked up and used by someone else.
	*/
	
	// malloc storage for pcp
	pcp = malloc(sizeof(*pcp));
	if (!pcp) {
		props->reserved_threads--;
		_mutex_unlock(&(props->inline_lock));
		return(-1);
	}
	ctp = pool->pool_attr.context_alloc(pool->pool_attr.handle);
	if (!ctp) {
    		props->reserved_threads--;
    		_mutex_unlock(&(props->inline_lock));
		// free storage for pcp
		free(pcp);
		return(-1);
	}
	// Initialize pcp
	pcp->ctp = ctp;
	pcp->pool = pool;
	pcp->flags = 0;

	pool->created++;
	pool->waiting++;
	_mutex_unlock(&(props->inline_lock));
  	ret = pthread_create(&tid, pool->pool_attr.attr, 
                       	_thread_pool_reserve_thread, pcp);
  	if(ret != 0) {
    		errno = ret;
    		ret = -1;
		// free storage for pcp
		free(pcp);
		_mutex_lock(&(props->inline_lock));
		pool->created--;
		pool->waiting--;
		pool->pool_attr.context_free(ctp);
    	props->reserved_threads--;
		_mutex_unlock(&(props->inline_lock));
		return(ret);
	}
	return(ret);
}

void *_thread_pool_reserve_thread(void *data) {
	struct _pool_context *pcp = data;
	thread_pool_t *pool = pcp->pool;
	void *ctp = pcp->ctp;

	(void)pthread_detach(pthread_self());

	// free storage for pcp before blocking in thread pool
	// this storage was created by the parent thread
	free(pcp);
	return(_thread_pool_thread(pool, ctp));
}

void *_thread_pool_context_thread(void *data) {
	thread_pool_t        *pool = data;
	void                 *ctp;
	struct _pool_properties *props;

	(void)pthread_detach(pthread_self());

	props = (struct _pool_properties *)pool->props;
	if(!(ctp = pool->pool_attr.context_alloc(pool->pool_attr.handle))) {
		_mutex_lock(&(props->inline_lock));
		pool->created--;
		props->newthreads--;
		_mutex_unlock(&(props->inline_lock));
		pthread_exit(0);
	}
	// call update, increment waiting, decrement newthreads
	_thread_pool_update(pool, 1, -1);

	return(_thread_pool_thread(pool, ctp));
}

void *_thread_pool_thread(thread_pool_t *pool, void *ctp) {
	struct _pool_context pcp;
	struct _pool_properties *props;
	void *old_ctp;

	props = (struct _pool_properties *)pool->props;

	pcp.pool = pool;
	pcp.ctp = ctp;
	pcp.flags = PCP_FLAG_WAITING;

	pthread_cleanup_push(&_thread_cleanup, &pcp);

	do {
		old_ctp = ctp;
		ctp = pool->pool_attr.block_func(ctp);

		pcp.flags &= ~PCP_FLAG_WAITING;

		// call update, decrement waiting, newthreads unchanged
		_thread_pool_update(pool, -1, 0);

		if (pool->pool_attr.handler_func(ctp) == -1) {
			ctp = old_ctp; 
			/* Fall thru so we update stats and potential exit */

			/* AM Note @@NYI, We should have a error handler callout so we can notify
				error conditions in a thread pool
			*/
		}

		_mutex_lock(&(props->inline_lock));
		// exit if we are over the high water mark _and_
		// over the low water mark also
		if ((pool->waiting >= (pool->pool_attr.hi_water + props->reserved_threads)) &&
        (pool->waiting >= (pool->pool_attr.lo_water + props->reserved_threads))) {
			break;
		}
		pool->waiting++;
		_mutex_unlock(&(props->inline_lock));
		pcp.flags |= PCP_FLAG_WAITING;
	} while (1);

	pthread_cleanup_pop(0);
	pool->created--;
	//Optimize on exit by only signalling once
	if(pool->flags & POOL_FLAG_CHANGING) {
		pthread_cond_signal(&(props->pool_cond));
	}
	_mutex_unlock(&(props->inline_lock));
	pool->pool_attr.context_free(pcp.ctp);

	pthread_exit(0);
	/* NOTREACHED */
	return NULL;
}

__SRCVERSION("_thread_pool_thread.c $Rev: 198661 $");
