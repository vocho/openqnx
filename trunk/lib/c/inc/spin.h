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
#include <sync.h>

#undef	ALLOW_SPIN_SMP		/* When faster smp spinlocks are coded */
#undef	OLD_SPIN			/* For backwards compatibility */

extern int _spin_init(spinlock_t *lock, int pshared);
extern int _spin_destroy(spinlock_t *lock);

#ifdef OLD_SPIN
extern int (*_spin_init_v)(struct _sync *__sync, const struct _sync_attr *attr);
extern int (*_spin_destroy_v)(struct _sync *__sync);
#endif
extern int _pthread_spin_init(pthread_spinlock_t *lock, int pshared);
extern int _pthread_spin_destroy(pthread_spinlock_t *lock);
extern int _pthread_spin_lock(pthread_spinlock_t *lock);
extern int _pthread_spin_trylock(pthread_spinlock_t *lock);
extern int _pthread_spin_unlock(pthread_spinlock_t *lock);

/* __SRCVERSION("spin.h $Rev: 153052 $"); */
