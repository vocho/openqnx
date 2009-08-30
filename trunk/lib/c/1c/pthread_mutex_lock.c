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
#include <sys/neutrino.h>
#include "cpucfg.h"

// This function must not be a cancellation point
// This function must not return EINTR

int
(pthread_mutex_lock)(pthread_mutex_t *mutex) {
	int			id = LIBC_TLS()->__owner;
	int			ret;

	// Is it priority ceiling?
	if(!(mutex->__count & _NTO_SYNC_PRIOCEILING)) {
		// Is it unlocked?
		if(__mutex_smp_cmpxchg(&mutex->__owner, 0, id) == 0) {
			++mutex->__count;
			return EOK;
		}
	}

	// Is it locked by me?
	if((mutex->__owner & ~_NTO_SYNC_WAITING) == id) {
		if((mutex->__count & _NTO_SYNC_NONRECURSIVE) == 0) {
			// We have it so bump the count.
			if((mutex->__count & _NTO_SYNC_COUNTMASK) == _NTO_SYNC_COUNTMASK) {
				return EAGAIN;	/* As per Unix 98 */
			}
			++mutex->__count;
			return EOK;
		}
		if((mutex->__count & _NTO_SYNC_NOERRORCHECK) == 0) {
			return EDEADLK;
		}
	}

	// Someone else owns it. Wait for it. Or, ceiling case, enter kernel.
	if((ret = SyncMutexLock_r((sync_t *)mutex)) != EOK) {
		return ret;
	}

	// We have it so bump the count.
	++mutex->__count;
	return EOK;
}

__SRCVERSION("pthread_mutex_lock.c $Rev: 153052 $");
