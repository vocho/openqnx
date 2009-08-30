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
 *  resource.h      
 *

 */
#ifndef __RESOURCE_H_INCLUDED
#define __RESOURCE_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef _SYS_TIME_H_
#include <sys/time.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if defined(__ID_T)
typedef __ID_T		id_t;
#undef __ID_T
#endif

#undef NZERO			/* Use sysconf(_SC_NZERO) */

#define	PRIO_PROCESS	0		/* Identifies who argument as a process ID. */
#define	PRIO_PGRP		1		/* Identifies who argument as a process group ID. */
#define	PRIO_USER		2		/* Identifies who argument as a user ID. */

#define	RLIMIT_CPU		0		/* Limit on CPU time per process. */
#define	RLIMIT_FSIZE	1		/* Limit on file size. */
#define	RLIMIT_DATA		2		/* Limit on data segment size. */
#define	RLIMIT_STACK	3		/* Limit on stack size. */
#define	RLIMIT_CORE		4		/* Limit on size of core dump file. */
#define	RLIMIT_NOFILE	5		/* Limit on number of open files. */
#define	RLIMIT_OFILE	RLIMIT_NOFILE
#define	RLIMIT_AS		6		/* Limit on address space size. */
#define	RLIMIT_VMEM		RLIMIT_AS
#define	RLIMIT_RSS		RLIMIT_AS
#define	RLIMIT_MEMLOCK	7		/* locked-in-memory address space */
#define	RLIMIT_NPROC	8		/* number of processes */
#define	RLIMIT_NTHR 	9		/* number of threads */

#define	RLIM_NLIMITS	10		/* number of resource limits */

#if _FILE_OFFSET_BITS - 0 == 64
typedef _Uint64t		rlim_t;
#define	RLIM_INFINITY	((rlim_t)-3)	/* A value of rlim_t indicating no limit. */
#define	RLIM_SAVED_MAX	((rlim_t)-2)	/* A value of type rlim_t indicating an unrepresentable saved hard limit. */
#define	RLIM_SAVED_CUR	((rlim_t)-1)	/* A value of type rlim_t indicating an unrepresentable saved soft limit. */
#else
typedef unsigned long	rlim_t;
#define RLIM_INFINITY   0x7fffffff		/* A value of rlim_t indicating no limit. */
#define RLIM_SAVED_MAX  0x7ffffffe		/* A value of type rlim_t indicating an unrepresentable saved hard limit. */
#define RLIM_SAVED_CUR  0x7ffffffd		/* A value of type rlim_t indicating an unrepresentable saved soft limit. */
#endif

#include <_pack64.h>

struct rlimit {
#if _FILE_OFFSET_BITS - 0 == 64
	rlim_t		rlim_cur;	/* the current (soft) limit */
	rlim_t		rlim_max;	/* hard limit */
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
#if defined(__LITTLEENDIAN__)
	rlim_t		rlim_cur;	/* the current (soft) limit */
	rlim_t		rlim_cur_hi;
	rlim_t		rlim_max;	/* hard limit */
	rlim_t		rlim_max_hi;
#elif defined(__BIGENDIAN__)
	rlim_t		rlim_cur_hi;
	rlim_t		rlim_cur;	/* the current (soft) limit */
	rlim_t		rlim_max_hi;
	rlim_t		rlim_max;	/* hard limit */
#else
 #error endian not configured for system
#endif
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
};

#if _LARGEFILE64_SOURCE - 0 > 0
typedef _Uint64t		rlim64_t;

#define	RLIM64_INFINITY		((rlim64_t)-3)
#define	RLIM64_SAVED_MAX	((rlim64_t)-2)
#define	RLIM64_SAVED_CUR	((rlim64_t)-1)

struct rlimit64 {
	rlim64_t	rlim_cur;	/* the current (soft) limit */
	rlim64_t	rlim_max;	/* hard limit */
};
#endif

#define	RUSAGE_SELF			0	/* Returns information about the current process. */
#define	RUSAGE_CHILDREN		-1	/* Returns information about children of the current process. */

struct	rusage {
	struct timeval	ru_utime;	/* user time used */
	struct timeval	ru_stime;	/* system time used */
	long			ru_maxrss;	/* max resident set size */
	long			ru_ixrss;	/* integral shared memory size */
#define	ru_first	ru_ixrss
	long			ru_idrss;	/* integral unshared data " */
	long			ru_isrss;	/* integral unshared stack " */
	long			ru_minflt;	/* page reclaims */
	long			ru_majflt;	/* page faults */
	long			ru_nswap;	/* swaps */
	long			ru_inblock;	/* block input operations */
	long			ru_oublock;	/* block output operations */
	long			ru_msgsnd;	/* messages sent */
	long			ru_msgrcv;	/* messages received */
	long			ru_nsignals;/* signals received */
	long			ru_nvcsw;	/* voluntary context switches */
	long			ru_nivcsw;	/* involuntary " */
#define	ru_last		ru_nivcsw
};

__BEGIN_DECLS

#if _LARGEFILE64_SOURCE - 0 > 0 || _FILE_OFFSET_BITS - 0 == 64
struct rlimit64;
extern int  getrlimit64(int, struct rlimit64 *);
extern int  setrlimit64(int, const struct rlimit64 *);
#endif
#if _FILE_OFFSET_BITS-0 == 64
#if defined(__WATCOMC__)
extern int  getrlimit(int, struct rlimit *);
#pragma aux getrlimit "getrlimit64";
extern int  setrlimit(int, const struct rlimit *);
#pragma aux setrlimit "setrlimit64";
#else
static __inline int __attribute__((__unused__)) getrlimit(int __t, struct rlimit *__r) {
	return getrlimit64(__t, (struct rlimit64 *)__r);
}
static __inline int __attribute__((__unused__)) setrlimit(int __t, const struct rlimit *__r) {
	return setrlimit64(__t, (const struct rlimit64 *)__r);
}
#endif
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
extern int  getrlimit(int, struct rlimit *);
extern int  setrlimit(int, const struct rlimit *);
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif

extern int  getpriority(int, id_t);
extern int  getrusage(int, struct rusage *);
extern int  setpriority(int, id_t, int);

#include <_packpop.h>

__END_DECLS

#endif

/* __SRCVERSION("resource.h $Rev: 153052 $"); */
