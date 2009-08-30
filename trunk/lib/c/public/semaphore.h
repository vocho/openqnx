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
 *  semaphore.h
 *

 */
#if !defined(_SEMAPHORE_H_INCLUDED) && !defined(__CYGWIN__)
#define _SEMAPHORE_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#if defined(__SYNC_T)
typedef __SYNC_T		sync_t;
#undef __SYNC_T
#endif
typedef sync_t			sem_t;

#if !defined(__EXT_POSIX1_199309) && (defined(__EXT_POSIX1_198808) || defined(__EXT_POSIX1_199009))
#error POSIX Semaphores needs P1003.1b-1993 or later
#endif

__BEGIN_DECLS

#include <_pack64.h>

#define SEM_FAILED      ((sem_t *)-1)

extern sem_t *sem_open(const char *__name, int __oflag, ...);
extern int sem_close(sem_t *__sem);
extern int sem_destroy(sem_t *__sem);
extern int sem_getvalue(sem_t *__sem, int *__value);
extern int sem_init(sem_t *__sem, int __pshared, unsigned __value);
extern int sem_post(sem_t *__sem);
extern int sem_trywait(sem_t *__sem);
extern int sem_unlink(const char *__name);
extern int sem_wait(sem_t *__sem);
#if defined(__EXT_POSIX1_200112)		/* Approved 1003.1d D14 */
struct timespec;
extern int sem_timedwait(sem_t *__sem, const struct timespec *__abs_timeout);
#endif

#include <_packpop.h>

__END_DECLS

#endif

/* __SRCVERSION("semaphore.h $Rev: 153052 $"); */
