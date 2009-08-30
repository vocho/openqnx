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
 *  sys/wait.h  Define system wait types
 *

 */
#ifndef __WAIT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

#if defined(__EXT_XOPEN_EX)

#ifndef _SIGNAL_H_INCLUDED
#include <signal.h>
#endif

#if defined(__ID_T)
typedef __ID_T		id_t;
#undef __ID_T
#endif

#endif

/*
 * Defined system wait types
 */
#define WUNTRACED	0x0004		/* Processes stopped by signals */
#define WNOHANG     0x0040		/* Do not block waiting */
#if defined(__EXT_XOPEN_EX) || defined(__EXT_UNIX_MISC)
#define WEXITED		0x0001		/* Wait for processes that have exited */
#define WSTOPPED	WUNTRACED
#define WCONTINUED	0x0008		/* Processes continued by signals */
#define WNOWAIT		0x0080		/* Don't free waited on child */
#endif
#if defined(__EXT_UNIX_MISC)
#define WTRAPPED	0x0002		/* Process stopped at debugger point of interest */
#define WOPTMASK	(WEXITED|WUNTRACED|WCONTINUED|WNOHANG|WNOWAIT|WTRAPPED)

#define WCONTFLG	0xffff
#define WCOREFLG	0x0080
#define WSTOPFLG	0x007f
#define WSIGMASK	0x007f
#endif

#if defined(__EXT_XOPEN_EX)
typedef enum {
	P_ALL,
	P_PID,
	P_PGID
} idtype_t;
#endif

/*
 * Define 1003.1 macros.
 */
#define WIFEXITED(__status)		(((__status) & 0xff) == 0)
#define WEXITSTATUS(__status)	(((__status) >> 8) & 0xff)
#define WIFSIGNALED(__status)	(((__status) & 0xff) != 0 && ((__status) & 0xff00) == 0)
#define WTERMSIG(__status)		((__status) & 0x7f /*WSIGMASK*/)
#define WIFSTOPPED(__status)	(((__status) & 0xff) == 0x7f /*WSTOPFLG*/ && ((__status) & 0xff00) != 0)
#define WSTOPSIG(__status)		(((__status) >> 8) & 0xff)
#if defined(__EXT_XOPEN_EX)
#define WIFCONTINUED(__status)	(((__status) & 0xffff) == 0xffff /*WCONTFLG*/)
#endif
#if defined(__EXT_UNIX_MISC)
#define WCOREDUMP(__status)		((__status) & WCOREFLG)
#endif
#if defined(__EXT_QNX) /* POSIX 1003.1d Draft 8 */
#define WIFSPAWNFAIL(__status)	0	/* spawn() always returns -1 and sets errno */
#define WSPAWNERRNO(stat_val)	0
#endif

/*
 *  POSIX 1003.1 Prototypes.
 */
__BEGIN_DECLS

extern pid_t wait(int *__stat_loc);
extern pid_t waitpid(pid_t __pid, int *__stat_loc, int __options);
#if defined(__EXT_XOPEN_EX)
struct rusage;
extern pid_t wait3(int *__stat_loc, int __options, struct rusage *__resource_usage);
extern pid_t wait4(pid_t __pid, int *__stat_loc, int __options, struct rusage *__resource_usage);
extern int waitid(idtype_t __idtype, id_t __id, siginfo_t *__infop, int __options);
extern int __waitid_net(int nd, idtype_t idtype, id_t id, siginfo_t *infop, int options);
#endif


__END_DECLS

#define __WAIT_H_INCLUDED
#endif

/* __SRCVERSION("wait.h $Rev: 153052 $"); */
