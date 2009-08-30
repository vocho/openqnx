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
#include <pthread.h>

int pthread_barrier_init(pthread_barrier_t *b, const pthread_barrierattr_t *cattr, unsigned int count) {
	int						status;
	struct _sync_attr		*attr, buff;

	/* invalidate the ADT */
	b->__barrier = b->__count = 0;

	/* validate the given attributes and the given count */
	if (count == 0) {
		return EINVAL;
	}

	/* Copy the attribute so we can make modifications */
	attr = 0;
	if(cattr) {
		/* validate the given attributes */
		if(cattr->__flags & (_NTO_ATTR_FLAGS & ~PTHREAD_PROCESSSHARED_MASK)) {
			/* only pshared is allowed */
			return EINVAL;
		}
		buff = *cattr;
		attr = &buff;
	}

	/* initialize the spin locks or mutexes */
	if(attr) {
		attr->__flags = (attr->__flags & PTHREAD_PROCESSSHARED_MASK) | _NTO_ATTR_MUTEX;
	}
	if ((status = pthread_mutex_init(&b->__lock, (pthread_mutexattr_t*)attr))) {
		return status;
	}

	if(attr) {
		attr->__flags = (attr->__flags & PTHREAD_PROCESSSHARED_MASK) | _NTO_ATTR_COND;
	}
	if ((status = pthread_cond_init(&b->__bcond, (pthread_condattr_t*)attr))) {
		pthread_mutex_destroy(&b->__lock);
		return status;
	}

	b->__barrier = b->__count = count;

	return EOK;
}

/* Only for binary compatibility with 2.0 programs */
#include <sync.h>

int (barrier_init)(barrier_t *b, const barrier_attr_t *attr, int count) {
	return pthread_barrier_init((pthread_barrier_t *)b, (pthread_barrierattr_t *)attr, count);
}

__SRCVERSION("barrier_init.c $Rev: 153052 $");
