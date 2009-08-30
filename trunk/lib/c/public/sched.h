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
 *  sched.h
 *

 */
#ifndef _SCHED_H_INCLUDED
#define _SCHED_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef _TIME_H_INCLUDED
 #include <time.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__EXT_POSIX1_199309) && (defined(__EXT_POSIX1_198808) || defined(__EXT_POSIX1_199009))
#error POSIX Scheduling needs P1003.1b-1993 or later
#endif

__BEGIN_DECLS

#include <_pack64.h>

#if defined(__EXT_QNX)
#define SCHED_NOCHANGE	0
#endif
#define SCHED_FIFO      1
#define SCHED_RR        2
#define SCHED_OTHER     3
#if defined(__EXT_POSIX1_200112)	/* Approved 1003.1d D14 */	
#define SCHED_SPORADIC	4
#endif
#if defined(__EXT_QNX)	
#define SCHED_ADJTOHEAD	5	/* Move to head of ready queue */
#define SCHED_ADJTOTAIL	6   /* Move to tail of ready queue */
#define SCHED_SETPRIO	7	/* pthread_setschedprio operation */
#define SCHED_MAXPOLICY 7	/* Maximum valid policy entry */

#endif

#if defined(__EXT_QNX)
#define SCHED_EXT_NONE	0
#define SCHED_EXT_APS	1	/* APS scheduller */

#define SCHED_EXT_CMD_BASE		  0
#define SCHED_EXT_APS_CMD_BASE	200

enum {
	SCHED_QUERY_SCHED_EXT = SCHED_EXT_CMD_BASE,
};
struct sched_query {
	_Uint32t		extsched;
	_Uint32t		reserved;
};
#endif

#if defined(__SCHED_PARAM_T)
typedef __SCHED_PARAM_T	sched_param_t;
#undef __SCHED_PARAM_T
#endif

#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
	#define sched_ss_low_priority	__sched_ss_low_priority
	#define sched_ss_max_repl		__sched_ss_max_repl
	#define sched_ss_repl_period	__sched_ss_repl_period
	#define sched_ss_init_budget	__sched_ss_init_budget
#endif

#if defined(__EXT_QNX)
/* 1003.4 Draft 9 : These two are so convenient we include them. */
extern int  getprio(pid_t __pid);
extern int  setprio(pid_t __pid, int __priority);
#endif

/* 1003.1b */
extern int sched_getparam(pid_t __pid, struct sched_param *__parms);
extern int sched_getscheduler(pid_t __pid);
extern int sched_setparam(pid_t __pid, const struct sched_param *__parms);
extern int sched_setscheduler(pid_t __pid, int __policy, const struct sched_param *__parms);
extern int sched_yield(void);
extern int sched_rr_get_interval(pid_t __pid, struct timespec *__t);
extern int sched_get_priority_min(int __alg);
extern int sched_get_priority_max(int __alg);
#if defined(__EXT_QNX)
extern int sched_get_priority_adjust(int __prio, int __alg, int __adjust);
#endif

#include <_packpop.h>

__END_DECLS

#if defined(__EXT_QNX) && defined(__INLINE_FUNCTIONS__)

#ifndef __NEUTRINO_H_INCLUDED
#include <sys/neutrino.h>
#endif

#define sched_yield SchedYield

#endif

#endif

/* __SRCVERSION("sched.h $Rev: 169304 $"); */
