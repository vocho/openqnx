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




#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/neutrino.h>
#include "cpucfg.h"
#include "sleepon.h"
	
extern int _sleepon_wait(sleepon_t *__handle, const volatile void *__addr, _uint64 nsec);
static int _sleepon_wakeup(sleepon_t *l, const volatile void *addr, int type);

#define FLAG_SIGNAL			0x10000000
#define FLAG_BROADCAST		0x20000000
#define FLAG_ACTIVE			0xf0000000
#define FLAG_COUNT_MASK		(~FLAG_ACTIVE)

#define SLEEPON_DYING			0x80000000

struct _sleepon_entry {
	struct _sleepon_entry		*next;
	struct _sleepon_entry		**prev;
	unsigned					flags;
	void						*addr;
	pthread_cond_t				cond;
	sleepon_t					*list;
};

struct _sleepon_handle _sleepon_default = {
	PTHREAD_MUTEX_INITIALIZER, NULL, NULL, 0, 1
};

int _sleepon_lock(sleepon_t *l) {
	return pthread_mutex_lock(&l->mutex);
}

int _sleepon_unlock(sleepon_t *l) {
	return pthread_mutex_unlock(&l->mutex);
}

int _sleepon_init(sleepon_t **pl, unsigned flags) {
	sleepon_t				*l;
	int						status;

	if(!(l = calloc(sizeof *l, 1))) {
		return ENOMEM;
	}
	if((status = pthread_mutex_init(&l->mutex, 0)) != EOK) {
		free(l);
		return status;
	}
	l->flags = flags;
	l->inuse = 1;
	*pl = l;
	return EOK;
}

static void _sleepon_done(sleepon_t *l) {
	struct _sleepon_entry	*p;

	// Don't allow the default sleepon list to be destroyed
	if(l == &_sleepon_default) { //should never happen (destroy checks before decrementing inuse)
		return;
	}

#ifndef NDEBUG
	assert(l->list == NULL);
#endif
	while((p = l->list)) {
		l->list = p->next;
		pthread_cond_destroy(&p->cond);
		free(p);
	}
	while((p = l->free)) {
		l->free = p->next;
		pthread_cond_destroy(&p->cond);
		free(p);
	}

	// Make the mutex invalid so we never use it again
	if(pthread_mutex_destroy(&l->mutex) == EBUSY) {
		/* This is for old kernels that don't allow a mutex owner to destroy it */
		pthread_mutex_unlock(&l->mutex);
		while(pthread_mutex_destroy(&l->mutex) != EOK) {
			/* nothing to do */
		}
	}

	free(l);
}

int _sleepon_destroy(sleepon_t *l) {
	int						status;
	struct _sleepon_entry	*p;
	int do_unlock = 0;

	// Don't allow the default sleepon list to be destroyed
	if(l == &_sleepon_default) {
		return EBUSY;
	}

	// If caller does not have the mutex locked, return and error
	if((l->mutex.__owner & ~_NTO_SYNC_WAITING) != LIBC_TLS()->__owner) {
		if((status = pthread_mutex_lock(&l->mutex)) != EOK) {
			return status;
		}
		do_unlock = 1;
	}

	if(l->inuse & SLEEPON_DYING) {
		if(do_unlock) {
			pthread_mutex_unlock(&l->mutex);
		}
		return EINPROGRESS;
	}

	// Now start everyone cleaning up
	if(--l->inuse == 0) {
		_sleepon_done(l);
	} else {
		l->inuse |= SLEEPON_DYING;
		for(p = l->list; p; p = p->next) {
			(void)_sleepon_wakeup(l, p->addr, FLAG_BROADCAST);
		}
	}
	return EOK;
}

static void _sleepon_cleanup(void *data) {
	struct _sleepon_entry	*p = data;
	sleepon_t				*l = p->list;

	// Is this the last waiting thread?
	if((--p->flags & FLAG_COUNT_MASK) == 0) {

		// remove the sleepon structure from the double linked list
		if((*p->prev = p->next)) {
			p->next->prev = p->prev;
		}

		// Add to the free list
		p->next = l->free;
		l->free = p;

	} else if((p->flags & FLAG_ACTIVE) == FLAG_SIGNAL) {

		// if not a broadcast.
		// re-signal the condvar to make sure another thread
		// will be awakened. (avoiding a dead lock)
		pthread_cond_signal(&p->cond);
	}

	// Now start everyone cleaning up
	if((--l->inuse & ~SLEEPON_DYING) == 0) {
		_sleepon_done(l);
	} else {
		// Unlock the mutex
		_sleepon_unlock(l);
	}
}

int _sleepon_wait(sleepon_t *l, const volatile void *addr, _uint64 nsec) {
	struct _sleepon_entry		*p, **pp;
	int							ret = EOK;

	// If caller does not have the mutex locked, return and error
	if((l->mutex.__owner & ~_NTO_SYNC_WAITING) != LIBC_TLS()->__owner || (l->inuse & SLEEPON_DYING)) {
		return EINVAL;
	}

	// Search for a pending wait that has the same address
	for(pp = &l->list; (p = *pp); pp = &p->next) {
		if((p->addr == addr) && !(p->flags & FLAG_BROADCAST)) {
			break;
		}
	}

	if(!p) {
		if((p = l->free)) {
			// get a sleepon structure from its free list
			l->free = p->next;
		} else {
			pthread_condattr_t			attr;	

			// Try to allocate a sleepon structure
			if(!(p = (struct _sleepon_entry *)malloc(sizeof *p))) {
				return ENOMEM;
			}
			
			if(((ret = pthread_condattr_init(&attr)) != EOK) 
					|| (ret = (pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) != EOK)) 
					|| ((ret = pthread_cond_init(&p->cond, &attr)) != EOK)) {
				(void)pthread_condattr_destroy(&attr);
				free(p);
				return ret;
			}
			(void)pthread_condattr_destroy(&attr);

			p->list = l;
		}

		// initialize the sleepon structure and add it to the head of the list
		p->addr = (void *)addr;
		p->flags = 0;
		p->prev = &l->list;
		if((p->next = l->list)) {
			p->next->prev = &p->next;
		}
		l->list = p;
	}

	// Increment the count of waiting threads
	p->flags++;
	l->inuse++;

	// setup a cleanup handler as cond_wait is a cancellation point.
	pthread_cleanup_push(_sleepon_cleanup, p);

	// Use a while loop as multiple threads could come out of cond_wait.
	if(nsec) {
		struct timespec		abstime;
		uint64_t			ct;

		(void)ClockTime_r(CLOCK_MONOTONIC, 0, &ct);
		nsec2timespec(&abstime, nsec + ct);
		while(!(p->flags & FLAG_ACTIVE)) {
			if(pthread_cond_timedwait(&p->cond, &l->mutex, &abstime) == ETIMEDOUT) {
				ret = ETIMEDOUT;
				break;
			}
		}
	} else {
		while(!(p->flags & FLAG_ACTIVE)) {
			pthread_cond_wait(&p->cond, &l->mutex);
		}
	}

	// no more cancelation points, so remove cleanup handler
	pthread_cleanup_pop(0);

	// Is this the last waiting thread?
	if((--p->flags & FLAG_COUNT_MASK) == 0) {

		// remove the sleepon structure from the double linked list
		if((*p->prev = p->next)) {
			p->next->prev = p->prev;
		}

		// Add to the free list
		p->next = l->free;
		l->free = p;

	} else if(!(p->flags & FLAG_BROADCAST)) {

		// Only clear the SIGNAL flag if not a broadcast
		p->flags &= ~FLAG_SIGNAL;
	}

	// Check for last use, if so clean up
	if((--l->inuse & ~SLEEPON_DYING) == 0) {
		_sleepon_done(l);
	}

	return ret;
}

static int
_sleepon_wakeup(sleepon_t *l, const volatile void *addr, int type) {
	struct _sleepon_entry		*p;

	// If caller does not have the mutex locked, return and error
	if((l->mutex.__owner & ~_NTO_SYNC_WAITING) != LIBC_TLS()->__owner || (l->inuse & SLEEPON_DYING)) {
		return EINVAL;
	}

	for(p = l->list; p; p = p->next) {
		if(p->addr == addr) {

			// If already signalled/broadcast, we called wakeup again before
			// another thread was awakened. Don't do it again.
			if(!(p->flags & type)) {
				p->flags |= type;

				if(type & FLAG_BROADCAST) {
					// Turn on signal as well, since a signal following
					// a broadcast ain't going to do anything more.
					p->flags |= FLAG_SIGNAL;

					return pthread_cond_broadcast(&p->cond);
				} else {

					// usually causes only one thread to wake up.
					return pthread_cond_signal(&p->cond);
				}
			}
			break;
		}
	}
	return EOK;
}

int _sleepon_signal(sleepon_t *l, const volatile void *addr) {
	return _sleepon_wakeup(l, addr, FLAG_SIGNAL);
}

int _sleepon_broadcast(sleepon_t *l, const volatile void *addr) {
	return _sleepon_wakeup(l, addr, FLAG_BROADCAST);
}


int (pthread_sleepon_lock)(void) {
	return pthread_mutex_lock(&_sleepon_default.mutex);
}

int (pthread_sleepon_unlock)(void) {
	return pthread_mutex_unlock(&_sleepon_default.mutex);
}

int (pthread_sleepon_wait)(const volatile void *addr) {
	return _sleepon_wait(&_sleepon_default, addr, 0);
}

int (pthread_sleepon_timedwait)(const volatile void *addr, _uint64 nsec) {
	return _sleepon_wait(&_sleepon_default, addr, nsec);
}

int (pthread_sleepon_signal)(const volatile void *addr) {
	return _sleepon_wakeup(&_sleepon_default, addr, FLAG_SIGNAL);
}

int (pthread_sleepon_broadcast)(const volatile void *addr) {
	return _sleepon_wakeup(&_sleepon_default, addr, FLAG_BROADCAST);
}

__SRCVERSION("sleepon.c $Rev: 153052 $");
