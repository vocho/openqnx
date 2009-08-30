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



/*
 *  sync.h
 *

 */
#ifndef _SYNC_H_INCLUDED
#define _SYNC_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef _PTHREAD_H_INCLUDED
 #include <pthread.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

#include <_pack64.h>


#if defined(__EXT_QNX)		/* not approved P1003.1j/D5 */
/* From posix 1003.1j D5 !!!!!!!!!!!!! USE pthread_* calls instead !!!!!!!!! */

/* Map these calls to the approved POSIX ones */

#define BARRIER_SERIAL_THREAD		PTHREAD_BARRIER_SERIAL_THREAD

#define BARRIER_INITIALIZER(__b)	PTHREAD_BARRIER_INITIALIZER(__b)

typedef pthread_barrierattr_t			barrier_attr_t;
typedef pthread_barrier_t				barrier_t;

#define barrier_attr_init(__a)				pthread_barrierattr_init(__a)
#define barrier_attr_destroy(__a)			pthread_barrierattr_destroy(__a)
#define barrier_attr_getpshared(__a, __f)	pthread_barrierattr_getpshared((__a), (__f))
#define barrier_attr_setpshared(__a, __f)	pthread_barrierattr_setpshared((__a), (__f))
#define barrier_destroy(__b)				pthread_barrier_destroy(__b)
#define barrier_init(__b, __a, __c)			pthread_barrier_init((__b), (__a), (__c))
#define barrier_wait(__b)					pthread_barrier_wait(__b)

typedef pthread_spinlock_t		spinlock_t;

#define spin_init(__l)			pthread_spin_init((__l), PTHREAD_PROCESS_SHARED)
#define spin_destroy(__l)		pthread_spin_destroy(__l)
#define spin_lock(__l)			pthread_spin_lock(__l)
#define spin_trylock(__l)		pthread_spin_trylock(__l)
#define spin_unlock(__l)		pthread_spin_unlock(__l)

#endif

#include <_packpop.h>

__END_DECLS

#endif

/* __SRCVERSION("sync.h $Rev: 153052 $"); */
