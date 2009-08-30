/*
 * $QNXtpLicenseC:
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
 *  sys/target_nto.h
 *

 *
 *  The only symbols made visible by this header are
 *  OS/compiler reserved symbols.
 */
#ifndef __TARGET_NTO_H_INCLUDED
#define __TARGET_NTO_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error sys/target_nto.h should not be included directly.
#endif

#ifndef _NULL
#define _NULL		0
#endif

#undef __ID_T
#define __ID_T		_INT32

#undef __PID_T
#define __PID_T		_INT32

#undef __UID_T
#define __UID_T		_INT32

#undef __GID_T
#define __GID_T		_INT32

#if defined (__cplusplus)
#undef __WCHAR_T
#define __WCHAR_T	wchar_t
#else
#ifndef __WCHAR_T
#define __WCHAR_T	_Uint32t
#endif
#endif

#undef __WINT_T
#define __WINT_T	_Wchart

#ifndef _WCMAX
#define _WCMAX		0x7fffffff
#endif

#if _WCMAX == 0x7fffffff
#define _WCMIN		0
#else
#define _WCMIN		(-_WCMAX - 1)
#endif

#undef __OFF_T
#undef __INO_T
#undef __BLKCNT_T
#undef __FSBLKCNT_T
#undef __FSFILCNT_T
#if _FILE_OFFSET_BITS - 0 == 64
#define __OFF_T			_Int64t
#define __INO_T			_Uint64t
#define __BLKCNT_T		_Uint64t
#define __FSBLKCNT_T	_Uint64t
#define __FSFILCNT_T	_Uint64t
#define __OFF_BITS__	64
#elif !defined _FILE_OFFSET_BITS || _FILE_OFFSET_BITS - 0 == 32
#define __OFF_T			_Int32t
#define __INO_T			_Uint32t
#define __BLKCNT_T		_Uint32t
#define __FSBLKCNT_T	_Uint32t
#define __FSFILCNT_T	_Uint32t
#define __OFF_BITS__	32
#else
#error _FILE_OFFSET_BITS value is unsupported
#endif

#undef __SOCKLEN_T
#define __SOCKLEN_T		_Int32t

#undef __SA_FAMILY_T
#define __SA_FAMILY_T		_Uint8t

#undef __OFF64_T
#define __OFF64_T		_Int64t

#undef __INO64_T
#define __INO64_T		_Uint64t

#undef __BLKCNT64_T
#define __BLKCNT64_T	_Uint64t

#undef __FSBLKCNT64_T
#define __FSBLKCNT64_T	_Uint64t

#undef __FSFILCNT64_T
#define __FSFILCNT64_T	_Uint64t

#undef __BLKSIZE_T
#define __BLKSIZE_T		_Uint32t

#undef __TIME_T
#define __TIME_T		_Uint32t

#undef __CLOCK_T
#define __CLOCK_T		_Uint32t

#undef __MODE_T
#define __MODE_T		_Uint32t

#undef __DEV__T
#define __DEV_T			_Uint32t

#undef __NLINK_T
#define __NLINK_T		_Uint32t

#undef __TIMER_T
#define __TIMER_T		int

#undef __CLOCKID_T
#define __CLOCKID_T		int

#undef __USECONDS_T
#define __USECONDS_T	_Uint32t

#undef __SUSECONDS_T
#define __SUSECONDS_T	_Int32t

#undef __KEY_T
#define __KEY_T			_Uint32t

#undef __PTHREAD_T
#define __PTHREAD_T		_Int32t

#undef __IOVEC_T
#define __IOVEC_T \
	struct iovec { \
		void *iov_base; \
		_Sizet iov_len; \
	}

#undef __MBSTATE_T
#define __MBSTATE_T \
	struct _Mbstatet {	/* state of a multibyte translation */ \
		_Wchart		_Wchar; \
		char		_State; \
		char		_Reserved[3]; \
	}

#undef __FPOS_T
#define __FPOS_T \
	struct _Fpost {		/* file position */ \
	_Off64t				_Off; \
	_Mbstatet			_Wstate; \
	}

#undef _FPOSOFF
#define _FPOSOFF(fp)	((fp)._Off)

/* Synchronization structures. */
/*
 * owner
 *  -1       Static initalized mutex which is auto created on SyncWait
 *  -2       Destroyed mutex
 *  -3       Named semaphore (the count is used as an fd)
 */
#undef __SYNC_T
#define __SYNC_T \
	struct _sync { \
		int			__count;		/* Count for recursive mutexs and semaphores */ \
		unsigned	__owner;		/* Thread id (valid for mutex only) */ \
	}

#undef __SYNC_ATTR_T
#define __SYNC_ATTR_T \
	struct _sync_attr { \
		int		__protocol; \
		int		__flags; \
		int		__prioceiling;	/* Not implemented */ \
		int		__clockid;		/* Condvars only */ \
		int		__reserved[4]; \
	}

/*
 * Used to define time specifications.
 */
#undef __TIMESPEC_DEF
#define __TIMESPEC_DEF(__name, __pref) \
	struct __name {						\
	    _CSTD time_t	__pref##_sec;	\
	    long			__pref##_nsec;	\
    }

#undef	__TIMESPEC
#define __TIMESPEC	__TIMESPEC_DEF(timespec, tv)
#undef	__TIMESPEC_INTERNAL
#define	__TIMESPEC_INTERNAL	__TIMESPEC_DEF(__timespec, __tv)


#undef __SCHED_PARAM_DEF
#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define __SCHED_PARAM_DEF(__name, __ts, __pref) \
	struct __name { \
		_INT32	__pref##_priority; \
		_INT32	__pref##_curpriority; \
		union {	\
			_INT32	__reserved[8]; \
			struct {	\
				_INT32	__ss_low_priority;	\
				_INT32	__ss_max_repl;	\
				struct __ts	__ss_repl_period;	\
				struct __ts	__ss_init_budget;	\
			}			__ss;	\
		}			__ss_un;	\
	}
	#define __sched_ss_low_priority	__ss_un.__ss.__ss_low_priority
	#define __sched_ss_max_repl		__ss_un.__ss.__ss_max_repl
	#define __sched_ss_repl_period	__ss_un.__ss.__ss_repl_period
	#define __sched_ss_init_budget	__ss_un.__ss.__ss_init_budget
#else
#define __SCHED_PARAM_DEF(__name, __ts, __pref) \
	struct __name { \
		_INT32	__pref##_priority; \
		_INT32	__pref##_curpriority; \
		_INT32	__spare[8]; \
	}
#endif

#undef	__SCHED_PARAM_INTERNAL
#define	__SCHED_PARAM_INTERNAL	__SCHED_PARAM_DEF(__sched_param, __timespec,__sched)
#undef	__SCHED_PARAM_T
#define	__SCHED_PARAM_T	__SCHED_PARAM_DEF(sched_param, timespec, sched)

/*
 * Used to define thread creation attributes.
 */
#undef __PTHREAD_ATTR_T
#define __PTHREAD_ATTR_T \
	struct _thread_attr { \
		int							__flags; \
		_Sizet						__stacksize; \
		void						*__stackaddr; \
		void						(*__exitfunc)(void *__status); \
		int							__policy; \
		struct __sched_param		__param; \
		unsigned					__guardsize; \
		unsigned					__prealloc; \
		int							__spare[2]; \
	}


/*
 * Signal structures
 */
#undef __STACK_T
#define __STACK_T				\
	struct {					\
		void		*ss_sp;		\
		_Sizet		ss_size;	\
		int			ss_flags;	\
	} 

#undef __SIGSET_T
#define __SIGSET_T		\
	struct {			\
		long __bits[2];	\
	}

#undef __UCONTEXT_T
#define __UCONTEXT_T	struct __ucontext_t

/* These are C99 types */

typedef _Int8t						_Intleast8t;
typedef _Uint8t						_Uintleast8t;
typedef _Int8t						_Intfast8t;
typedef _Uint8t						_Uintfast8t;

typedef _Int16t						_Intleast16t;
typedef _Uint16t					_Uintleast16t;
typedef _Int16t						_Intfast16t;
typedef _Uint16t					_Uintfast16t;

typedef _Int32t						_Intleast32t;
typedef _Uint32t					_Uintleast32t;
typedef _Int32t						_Intfast32t;
typedef _Uint32t					_Uintfast32t;

typedef _Int64t						_Intleast64t;
typedef _Uint64t					_Uintleast64t;
typedef _Int64t						_Intfast64t;
typedef _Uint64t					_Uintfast64t;

typedef _Int64t						_Intmaxt;
typedef _Uint64t					_Uintmaxt;

#undef __PTHREAD_KEY_T
#define __PTHREAD_KEY_T		int

#undef __PTHREAD_COND_T
#define __PTHREAD_COND_T		struct _sync

#undef __PTHREAD_CONDATTR_T
#define __PTHREAD_CONDATTR_T	struct _sync_attr

#undef __PTHREAD_MUTEX_T
#define __PTHREAD_MUTEX_T		struct _sync

#undef __PTHREAD_MUTEXATTR_T
#define __PTHREAD_MUTEXATTR_T	struct _sync_attr

#undef __PTHREAD_ONCE_T
#define __PTHREAD_ONCE_T	\
	struct _pthread_once {	\
		int			__once;	\
		sync_t		__mutex;	\
	}

/*
 * Copyright (c) 1995-1999 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

/* Customize DINKUM libraries */
#if defined(__LITTLEENDIAN__)
#define _D0		3	/* 0: big endian, 3: little endian floating-point */
#elif defined(__BIGENDIAN__)
#define _D0		0	/* 0: big endian, 3: little endian floating-point */
#else
#error ENDIAN Not defined for system
#endif
#if __LONGDOUBLE_BITS__ == 64
#define _DLONG	0		/* 0: 64, 1: 80, 2: 128 long double bits */
#define _LBIAS	0x3fe	/* 64 long double bits */
#define _LOFF	4		/* 64 long double bits */
#elif __LONGDOUBLE_BITS__ == 80
#define _DLONG	1		/* 0: 64, 1: 80, 2: 128 long double bits */
#define _LBIAS	0x3ffe	/* 80/128 long double bits */
#define _LOFF	15		/* 80/128 long double bits */
#elif __LONGDOUBLE_BITS__ == 128
#define _DLONG	2		/* 0: 64, 1: 80, 2: 128 long double bits */
#define _LBIAS	0x3ffe	/* 80/128 long double bits */
#define _LOFF	15		/* 80/128 long double bits */
#error __LONGDOUBLE_BITS__ not a supported size
#endif

		/* FLOATING-POINT PROPERTIES */
#define _DBIAS	0x3fe	/* IEEE format double and float */
#define _DOFF	4
#define _FBIAS	0x7e
#define _FOFF	7
#define _FRND	1

		/* integer properties */
#define _BITS_BYTE	8
#define _C2			1	/* 0 if not 2's complement */
#if defined(__CHAR_SIGNED__)
#define _CSIGN		1	/* 0 if char is not signed */
#elif defined(__CHAR_UNSIGNED__)
#define _CSIGN		0	/* 0 if char is not signed */
#endif
#define _MBMAX		8	/* MB_LEN_MAX */

#define _MAX_EXP_DIG	8	/* for parsing numerics */
#define _MAX_INT_DIG	32
#define _MAX_SIG_DIG	48

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	#define	__SECSTR(__sec, __s) __asm__(".section " #__sec ";.asciz \"" __s "\";.previous")
#endif


#endif

/* __SRCVERSION("target_nto.h $Rev: 173311 $"); */
