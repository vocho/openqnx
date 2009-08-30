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




#include <pthread.h>
#include <errno.h>

int pthread_rwlock_init(pthread_rwlock_t *l, const pthread_rwlockattr_t *cattr)
{
	int						status;
	struct _sync_attr		*attr, buff;

	/* invalidate the ADT */
	l->__active = -9;

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

	if(attr) {
		attr->__flags = (attr->__flags & PTHREAD_PROCESSSHARED_MASK) | _NTO_ATTR_MUTEX;
	}
	if ((status = pthread_mutex_init(&l->__lock, (pthread_mutexattr_t *)attr))) {
		return status;
	}

	if(attr) {
		attr->__flags = (attr->__flags & PTHREAD_PROCESSSHARED_MASK) | _NTO_ATTR_COND;
	}
	if ((status = pthread_cond_init(&l->__rcond, (pthread_condattr_t *)attr))) {
		pthread_mutex_destroy(&l->__lock);
		return status;
	}

	if ((status = pthread_cond_init(&l->__wcond, (pthread_condattr_t *)attr))) {
		pthread_mutex_destroy(&l->__lock);
		pthread_cond_destroy(&l->__rcond);
		return status;
	}

	l->__owner = -2U;
	l->__active = l->__blockedwriters = l->__blockedreaders = l->__heavy = 0;

	return EOK;
}

__SRCVERSION("pthread_rwlock_init.c $Rev: 153052 $");
