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




#include <sys/syspage.h>
#include "spin.h"

int _spin_init(spinlock_t *lock, int pshared) {
	pthread_mutexattr_t			attr;
	int							status;

#ifdef ALLOW_SPIN_SMP
	if(_syspage_ptr->num_cpu > 1) {
		if(_spin_lock_v == pthread_mutex_lock) {
			_spin_lock_v = _pthread_spin_lock;
			_spin_trylock_v = _pthread_spin_trylock;
			_spin_unlock_v = _pthread_spin_unlock;
		}
		return _pthread_spin_init(lock, pshared);
	}
#endif

	if((status = pthread_mutexattr_init(&attr)) != EOK) {
		return status;
	}
	if((status = pthread_mutexattr_setpshared(&attr, pshared)) != EOK) {
		return status;
	}
	status = pthread_mutex_init((pthread_mutex_t *)lock, &attr);
	(void)pthread_mutexattr_destroy(&attr);
	return status;
}

__SRCVERSION("_spin_init.c $Rev: 153052 $");
