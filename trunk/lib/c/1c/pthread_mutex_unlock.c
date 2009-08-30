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
#include <hw/inout.h>
#include "cpucfg.h"

int
(pthread_mutex_unlock)(pthread_mutex_t *mutex) {
	if((mutex->__owner & ~_NTO_SYNC_WAITING) == LIBC_TLS()->__owner) {
		if(--mutex->__count <= 0 || (mutex->__count & _NTO_SYNC_COUNTMASK) == 0) {
			mem_barrier();
			if(__mutex_smp_xchg(&mutex->__owner, 0) & _NTO_SYNC_WAITING || mutex->__count & _NTO_SYNC_PRIOCEILING) {
				return SyncMutexUnlock_r((sync_t *)mutex);
			}
		}
		return EOK;
	}
	return EPERM;
}

__SRCVERSION("pthread_mutex_unlock.c $Rev: 153052 $");
