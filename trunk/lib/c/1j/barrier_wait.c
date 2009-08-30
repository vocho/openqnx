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

#define _BARRIER_FLAG	(~0u ^ (~0u >> 1))

int pthread_barrier_wait(pthread_barrier_t *b) {
	int rc, flag;

	if ((rc = pthread_mutex_lock(&b->__lock)) == EOK) {
		/* the upper bit of "barrier" is flag */
		flag = b->__barrier & _BARRIER_FLAG;

		if (--b->__count == 0) {
			/* change the "barrier" flag and wakeup all blocked threads */
			b->__count = b->__barrier & ~_BARRIER_FLAG;
			b->__barrier = (b->__barrier & _BARRIER_FLAG) ?
				(b->__barrier & ~_BARRIER_FLAG) : (b->__barrier | _BARRIER_FLAG);
			rc = (pthread_cond_broadcast(&b->__bcond) == EOK) ?
				PTHREAD_BARRIER_SERIAL_THREAD : rc;
		} else {
			/* wait until the "barrier" flag changes */
			while ((rc == EOK) && (flag == (b->__barrier & _BARRIER_FLAG))) {
				rc = pthread_cond_wait(&b->__bcond, &b->__lock);
			}
		}

		pthread_mutex_unlock(&b->__lock);
	}

	return rc;
}

/* Only for binary compatibility with 2.0 programs */
#include <sync.h>

int (barrier_wait)(barrier_t *b) {
	return pthread_barrier_wait((pthread_barrier_t *)b);
}

__SRCVERSION("barrier_wait.c $Rev: 153052 $");
