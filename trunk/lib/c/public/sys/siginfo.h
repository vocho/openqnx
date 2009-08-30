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
 *  sys/siginfo.h    Signal definitions
 *

 */
#ifndef __SIGINFO_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define __SIGINFO_H_INCLUDED
#endif

#ifndef __SIGINFO_H_DECLARED
#define __SIGINFO_H_DECLARED

_STD_BEGIN

#if defined(__CLOCK_T)
typedef __CLOCK_T	clock_t;
#undef __CLOCK_T
#endif

#if defined(__TIME_T)
typedef __TIME_T	time_t;
#undef __TIME_T
#endif

_STD_END

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

#if defined(__UID_T)
typedef __UID_T		uid_t;
#undef __UID_T
#endif

#if defined(__PTHREAD_ATTR_T)
#if defined(__TIMESPEC_INTERNAL)
__TIMESPEC_INTERNAL;
#undef __TIMESPEC_INTERNAL
#endif
#if defined(__SCHED_PARAM_INTERNAL)
__SCHED_PARAM_INTERNAL;
#undef __SCHED_PARAM_INTERNAL
#endif
typedef __PTHREAD_ATTR_T	pthread_attr_t;
#undef __PTHREAD_ATTR_T
#endif

#define SIGEV_NONE				0	/* notify */
#define SIGEV_SIGNAL			1	/* notify, signo, value */
#define SIGEV_SIGNAL_CODE		2	/* notify, signo, value, code */
#define SIGEV_SIGNAL_THREAD		3	/* notify, signo, value, code */
#define SIGEV_PULSE				4	/* notify, coid, priority, code, value */
#define SIGEV_UNBLOCK			5	/* notify */
#define SIGEV_INTR				6	/* notify */
#define SIGEV_THREAD			7	/* notify, notify_function, notify_attributes */

#include <_pack64.h>

union sigval {
    int         sival_int;
    void       *sival_ptr;
};

struct sigevent {
	int				sigev_notify;
#if defined(__WATCOMC__) && !defined(NO_EXT_KEYS)
	union {
		int				sigev_signo;
		int				sigev_coid;
		int				sigev_id;
		void			(*sigev_notify_function)(union sigval);
	};
#else
	union {
		int				__sigev_signo;
		int				__sigev_coid;
		int				__sigev_id;
		void			(*__sigev_notify_function)(union sigval);
	}				__sigev_un1;
#define sigev_signo				__sigev_un1.__sigev_signo
#define sigev_coid				__sigev_un1.__sigev_coid
#define sigev_id				__sigev_un1.__sigev_id
#define sigev_notify_function	__sigev_un1.__sigev_notify_function
#endif
	union sigval	sigev_value;
#if defined(__WATCOMC__) && !defined(NO_EXT_KEYS)
	union {
		struct {
			short			sigev_code;
			short			sigev_priority;
		};
		pthread_attr_t	*sigev_notify_attributes;
	};
#else
	union {
		struct {
			short			__sigev_code;
			short			__sigev_priority;
		} __st;
		pthread_attr_t	*__sigev_notify_attributes;
	}				__sigev_un2;
#define sigev_notify_attributes	__sigev_un2.__sigev_notify_attributes
#define sigev_code				__sigev_un2.__st.__sigev_code
#define sigev_priority			__sigev_un2.__st.__sigev_priority
#endif
};


#ifdef __EXT_QNX

/*
 * Make some nice macros for initializing the sigevent structure
 */

#define SIGEV_NONE_INIT(__e) (	\
	(__e)->sigev_notify = SIGEV_NONE)

#define SIGEV_SIGNAL_INIT(__e, __s)	(\
	(__e)->sigev_notify = SIGEV_SIGNAL, \
	(__e)->sigev_signo = (__s))

#define SIGEV_SIGNAL_VALUE_INIT(__e, __s, __v)	(\
	(__e)->sigev_notify = SIGEV_SIGNAL, \
	(__e)->sigev_signo = (__s),\
	(__e)->sigev_value.sival_ptr = (void *)(__v))

#define SIGEV_SIGNAL_CODE_INIT(__e, __s, __v, __c)	(\
	(__e)->sigev_notify = SIGEV_SIGNAL_CODE, \
	(__e)->sigev_signo = (__s), \
	(__e)->sigev_value.sival_ptr = (void *)(__v), \
	(__e)->sigev_code = (__c))

#define SIGEV_SIGNAL_THREAD_INIT(__e, __s, __v, __c)	(\
	(__e)->sigev_notify = SIGEV_SIGNAL_THREAD, \
	(__e)->sigev_signo = (__s), \
	(__e)->sigev_value.sival_ptr = (void *)(__v), \
	(__e)->sigev_code = (__c))

#define SIGEV_PULSE_INIT(__e, __f, __p, __c, __v)	(\
	(__e)->sigev_notify = SIGEV_PULSE, \
	(__e)->sigev_coid = (__f), \
	(__e)->sigev_priority = (__p), \
	(__e)->sigev_value.sival_ptr = (void *)(__v), \
	(__e)->sigev_code = (__c))

#define SIGEV_UNBLOCK_INIT(__e) (	\
	(__e)->sigev_notify = SIGEV_UNBLOCK)

#define SIGEV_INTR_INIT(__e) (	\
	(__e)->sigev_notify = SIGEV_INTR)

#define SIGEV_THREAD_INIT(__e, __f, __v, __a) (	\
	(__e)->sigev_notify = SIGEV_THREAD, \
	(__e)->sigev_notify_function = (__f), \
	(__e)->sigev_value.sival_ptr = (void *)(__v), \
	(__e)->sigev_notify_attributes = (__a))


#define SIGEV_TYPE_MASK		0x000000ff
#define SIGEV_FLAG_CRITICAL	0x00000100


/* Note some of bits in .sigev_notify have hidden meaning. So after initializing an event, do not directly assign
 * to .sigev_notify, instead, use this macro:
 */
#define SIGEV_SET_TYPE(__e, new_type) (__e)->sigev_notify = ((__e)->sigev_notify & ~SIGEV_TYPE_MASK) | new_type;


/* Also, to test the type of a sigevent, don't compare .SIGEV_NOTIFY, as some hidden bits may be set,
 * instead use SIGEV_GET_TYPE, example if( SIGEV_GET_TYPE(my_event) == SIGEV_PULSE) ...
 */
#define SIGEV_GET_TYPE(__e)  ((__e)->sigev_notify & SIGEV_TYPE_MASK)


/* to make threads use "critical time" as defined by the optional Adaptive Partitioning scheduler, set
 * a sigevent to be critical with:
 */
#define SIGEV_MAKE_CRITICAL(__e)	((__e)->sigev_notify |= SIGEV_FLAG_CRITICAL)
#define SIGEV_CLEAR_CRITICAL(__e)   ((__e)->sigev_notify &= ~SIGEV_FLAG_CRITICAL)


/*
 * for SIGEV_PULSE don't modify the receiving threads priority
 * when the pulse is received
 */
#define SIGEV_PULSE_PRIO_INHERIT		(-1)

#endif


#define SI_FROMUSER(sip)	((sip)->si_code <= 0)
#define SI_FROMKERNEL(sip)	((sip)->si_code > 0)

/*
 * if SI_FROMUSER() the following codes are used
 * si_pid == sending pid, si_uid == real uid of signaling process)
 */
#define SI_USER				0
#define SI_RESERVED1		(-1)
#define SI_QUEUE			(-2)
#define SI_TIMER			(-3)
#define SI_ASYNCIO			(-4)
#define SI_MESGQ			(-5)
#define SI_NOTIFY			(-6)
#define SI_IRQ				(-7)
/*
 * QNX managers will never use this range
 * except if returning extended notify
 * events (_NOTIFY_CONDE*) at a client's
 * request in response to an _IO_NOTIFY
 * message.
 */
#define SI_MINAVAIL			(-128)
#define SI_MAXAVAIL			(-64)

#define SI_NOINFO			127
#define SI_MAXSZ			126

/*
 * SIGILL codes (si_addr == address of faulting instruction)
 */
#define ILL_ILLOPC	1	/* Illegal opcode */
#define ILL_ILLOPN	2	/* Illegal operand */
#define ILL_ILLADR	3	/* Illegal addressing mode */
#define ILL_ILLTRP	4	/* Illegal trap */
#define ILL_PRVOPC	5	/* Privileged opcode */
#define ILL_PRVREG	6	/* Privileged register */
#define ILL_COPROC	7	/* Coprocessor error */
#define ILL_BADSTK	8	/* internal stack error */
#if defined(__EXT_QNX)
#define NSIGILL		8
#endif

/*
 * SIGSEGV codes (si_addr == address of faulting memory reference)
 */
#define SEGV_MAPERR	1	/* Address not mapped */
#define SEGV_ACCERR	2	/* No permissions */
#define SEGV_STKERR	3	/* ?? Stack exception */
#define SEGV_GPERR	4	/* ?? General protection */
#define SEGV_IRQERR	5	/* ?? Interrupt handler fault */
#if defined(__EXT_QNX)
#define NSIGSEGV	5
#endif

/*
 * SIGBUS codes (si_addr == address of faulting memory reference)
 */
#define BUS_ADRALN	1	/* Invalid address alignment */
#define BUS_ADRERR	2	/* Non-existant physical address */
#define BUS_OBJERR	3	/* Object specific hardware error (e.g. NMI parity error) */
#if defined(__EXT_QNX)
#define NSIGBUS		3
#endif

/*
 * SIGCHILD codes (si_pid == child pid, si_status == exit value or signal, si_uid == real uid of signaling process)
 */
#define CLD_EXITED	1	/* Child has exited (si_status == exit value) */
#define CLD_KILLED	2	/* Child was killed */
#define CLD_DUMPED	3	/* Child terminated abnormally */
#define CLD_TRAPPED	4	/* Traced child has trapped */
#define CLD_STOPPED	5	/* Child has stopped */
#define CLD_CONTINUED 6	/* Stopped child had continued */
#if defined(__EXT_QNX)
#define NSIGCLD		6
#endif

/*
 * SIGTRAP codes
 */
#define TRAP_BRKPT	1	/* Break Point */
#define TRAP_TRACE	2	/* Trace */
#if defined(__EXT_QNX)
#define NSIGTRAP	2
#endif

/*
 * SIGFPE signal codes (si_addr == address of faulting instruction)
 */
#define	FPE_INTDIV	1	/* Integer divide by zero */
#define	FPE_INTOVF	2	/* Integer overflow */
#define	FPE_FLTDIV	3	/* Floating point divide by zero */
#define	FPE_FLTOVF	4	/* Floating point overflow */
#define	FPE_FLTUND	5	/* Floating point underflow */
#define	FPE_FLTRES	6	/* Floating point inexact result */
#define	FPE_FLTINV	7	/* Invalid floating point operation */
#define	FPE_FLTSUB	8	/* Subscript out of range */
#define FPE_NOFPU	9	/* ?? No FPU or emulator */
#define FPE_NOMEM	10	/* ?? No kernel space for FPU save area */
#if defined(__EXT_QNX)
#define	NSIGFPE		10
#endif

typedef struct {
	int				si_signo;
	int				si_code;		/* if SI_NOINFO, only si_signo is valid */
	int				si_errno;
	union {
		int				__pad[7];
		struct {
			pid_t			__pid;
			union {
				struct {
					uid_t			__uid;
					union sigval	__value;
				}				__kill;		/* si_code <= 0 SI_FROMUSER */
				struct {
					_CSTD clock_t	__utime;
					int				__status;	/* CLD_EXITED status, else signo */
					_CSTD clock_t	__stime;
				}				__chld;		/* si_signo=SIGCHLD si_code=CLD_* */
			}				__pdata;
		}				__proc;
		struct {
			int				__fltno;
			void			*__fltip;
			void			*__addr;
			int				__bdslot;
		}				__fault;				/* si_signo=SIGSEGV,ILL,FPE,TRAP,BUS */
	}				__data;
}				siginfo_t;
#define si_pid		__data.__proc.__pid
#define si_value	__data.__proc.__pdata.__kill.__value
#define si_uid		__data.__proc.__pdata.__kill.__uid
#define si_status	__data.__proc.__pdata.__chld.__status
#define si_utime	__data.__proc.__pdata.__chld.__utime
#define si_stime	__data.__proc.__pdata.__chld.__stime
#define si_fltno	__data.__fault.__fltno
#define si_trapno	si_fltno
#define si_addr		__data.__fault.__addr
#define si_fltip	__data.__fault.__fltip
#define si_bdslot	__data.__fault.__bdslot

#include <_packpop.h>

#endif

#ifdef _STD_USING
using std::clock_t; using std::time_t;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("siginfo.h $Rev: 153052 $"); */
