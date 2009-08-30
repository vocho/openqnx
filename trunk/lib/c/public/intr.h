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
 *  intr.h    Interrupt handler definitions
 *

 */

#ifndef _INTR_H_INCLUDED
#define _INTR_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef _TIME_H_INCLUDED
 #include <time.h>
#endif

#ifndef _SIGNAL_H_INCLUDED
 #include <signal.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef int intr_t;

#define _POSIX_INTR_CONNECT_MAX			32

#define INTR_HANDLED_NOTIFY				0
#define INTR_HANDLED_DO_NOT_NOTIFY		1
#define INTR_NOT_HANDLED				2

__BEGIN_DECLS

extern int intr_capture(intr_t __intr, int (*intr_handler)(volatile void *__area), volatile void *__area, size_t __areasize);
extern int intr_lock(intr_t __intr);
extern int intr_release(intr_t __intr, int (*intr_handler)(volatile void *__area));
extern int intr_timed_wait(int __flags, const struct timespec *__timeout);
extern int intr_unlock(intr_t __intr);

__END_DECLS

#endif

/* __SRCVERSION("intr.h $Rev: 153052 $"); */
