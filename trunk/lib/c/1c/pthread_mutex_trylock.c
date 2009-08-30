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

int
(pthread_mutex_trylock)(pthread_mutex_t *mutex) {
	unsigned		owner, id = LIBC_TLS()->__owner;
	int				ret;

	// Is it unlocked?
	if((owner = __mutex_smp_cmpxchg(&mutex->__owner, 0, id)) == 0) {
		++mutex->__count;
		return EOK;
	}

	// Is it recursive and locked by me?
	if((mutex->__count & _NTO_SYNC_NONRECURSIVE) == 0) {
		if((owner & ~_NTO_SYNC_WAITING) == id) {
			++mutex->__count;
			return EOK;
		}
	}

	// Static intialized. Do an immediate tineout so no threads block.
	if(owner == _NTO_SYNC_INITIALIZER) {
		(void)TimerTimeout_r(CLOCK_REALTIME, _NTO_TIMEOUT_MUTEX, NULL, NULL, NULL);
		if((ret = SyncMutexLock_r((sync_t *)mutex)) != EOK) {
			return ((ret == ETIMEDOUT) ? EBUSY : ret);
		}

		++mutex->__count;
		return EOK;
	}

	if(mutex->__owner == _NTO_SYNC_DESTROYED) {
		return EINVAL;
	}

	return EBUSY;
}

__SRCVERSION("pthread_mutex_trylock.c $Rev: 153052 $");
