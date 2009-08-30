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
 *  sys/time.h    UNIX98 time types
 *

 */
#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SELECT_H_INCLUDED
#include <sys/select.h>
#endif

#if defined(__SUSECONDS_T)
typedef __SUSECONDS_T suseconds_t;
#undef __SUSECONDS_T
#endif

_C_STD_BEGIN

#if defined(__TIME_T)
typedef __TIME_T	time_t;
#undef __TIME_T
#endif

_C_STD_END

#include <_pack64.h>

struct timeval {
	_CSTD time_t	tv_sec;		/* seconds */
    suseconds_t		tv_usec;	/* microseconds */
};

#if defined(__EXT_UNIX_MISC)
/* Operations on timevals. */
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)						\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_usec cmp (uvp)->tv_usec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))
#define	timeradd(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;	\
		if ((vvp)->tv_usec >= 1000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_usec -= 1000000;			\
		}							\
	} while (0)
#define	timersub(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;	\
		if ((vvp)->tv_usec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_usec += 1000000;			\
		}							\
	} while (0)
#endif

struct  itimerval {
	struct timeval	it_interval;	/* timer interval */
	struct timeval	it_value;		/* current value */
};

#if defined(__EXT_QNX)
/*
 * This is only here as source code porting aid. The only function that
 * took these was gettimeofday (the second parameter) and Unix98 says
 * that should be a NULL pointer now.
 */
#if defined(__SLIB_DATA_INDIRECT) && !defined(__SLIB)
struct _timezone 
#else
struct timezone 
#endif
{
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime; /* type of dst correction */
};
#else
struct timezone;
#endif

#include <_packpop.h>

/*
 * for the which argument of getitimer() and setitimer()
 */
#define ITIMER_REAL		0
#define ITIMER_VIRTUAL	1
#define ITIMER_PROF		2

__BEGIN_DECLS
extern int getitimer (int __which, struct itimerval *__value);
extern int gettimeofday (struct timeval *__tp, void *__tzp);
extern int setitimer (int __which, const struct itimerval *__value, struct itimerval *__ovalue);
extern int settimeofday(const struct timeval *__tp, const struct timezone *__tzp);
extern int utimes (const char *__path, const struct timeval *__times);
__END_DECLS

#endif /* !_SYS_TIME_H_ */

/* __SRCVERSION("time.h $Rev: 153052 $"); */
