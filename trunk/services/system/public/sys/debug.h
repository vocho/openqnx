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

#ifndef __DEBUG_H_INCLUDED
#define __DEBUG_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if	!defined(_DEBUG_TARGET_ALL) \
 && !defined(_DEBUG_TARGET_X86) \
 && !defined(_DEBUG_TARGET_PPC) \
 && !defined(_DEBUG_TARGET_MIPS) \
 && !defined(_DEBUG_TARGET_ARM)
	#if defined(__X86__)
		#define _DEBUG_TARGET_X86
	#elif defined(__PPC__)
		#define _DEBUG_TARGET_PPC
	#elif defined(__MIPS__)
		#define _DEBUG_TARGET_MIPS
	#elif defined(__SH__)
		#define _DEBUG_TARGET_SH
	#elif defined(__ARM__)
		#define _DEBUG_TARGET_ARM
	#else
		#error not configured for system
	#endif
#endif

#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_X86)
#ifndef __X86_CONTEXT_H_INCLUDED
#include _NTO_HDR_(x86/context.h)
#endif
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_PPC)
#ifndef __PPC_CONTEXT_H_INCLUDED
#include _NTO_HDR_(ppc/context.h)
#endif
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_MIPS)
#ifndef __MIPS_CONTEXT_H_INCLUDED
#include _NTO_HDR_(mips/context.h)
#endif
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_SH)
#ifndef __SH_CONTEXT_H_INCLUDED
#include _NTO_HDR_(sh/context.h)
#endif
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_ARM)
#ifndef __ARM_CONTEXT_H_INCLUDED
#include _NTO_HDR_(arm/context.h)
#endif
#endif

#ifndef __FAULT_H_INCLUDED
#include _NTO_HDR_(sys/fault.h)
#endif

#ifndef _SIGNAL_H_INCLUDED
#include _NTO_HDR_(signal.h)
#endif

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

#if defined(__PTHREAD_T)
typedef __PTHREAD_T	pthread_t;
#undef __PTHREAD_T
#endif

#if defined(__SIGSET_T)
typedef __SIGSET_T	sigset_t;
#undef __SIGSET_T
#endif

#if defined(__UID_T)
typedef __UID_T		uid_t;
#undef __UID_T
#endif

#if defined(__GID_T)
typedef __GID_T		gid_t;
#undef __GID_T
#endif

#if defined(__ITIMER)
struct _itimer __ITIMER;
#undef __ITIMER
#endif

#if defined(__TIMER_T)
typedef __TIMER_T timer_t;
#undef __TIMER_T
#endif

#if defined(__CLOCKID_T)
typedef __CLOCKID_T clockid_t;
#undef __CLOCKID_T
#endif

#if defined(__TIMER_INFO)
struct _timer_info __TIMER_INFO;
#undef __TIMER_INFO
#endif

__BEGIN_DECLS

#include _NTO_HDR_(_pack64.h)

#define _DEBUG_FLAG_STOPPED			0x00000001	/* Thread is not running */
#define _DEBUG_FLAG_ISTOP			0x00000002	/* Stopped at point of interest */
#define _DEBUG_FLAG_IPINVAL			0x00000010	/* IP is not valid */
#define _DEBUG_FLAG_ISSYS			0x00000020	/* System process */
#define _DEBUG_FLAG_SSTEP			0x00000040	/* Stopped because of single step */
#define _DEBUG_FLAG_CURTID			0x00000080	/* Thread is current thread */
#define _DEBUG_FLAG_TRACE_EXEC		0x00000100	/* Stopped because of breakpoint */
#define _DEBUG_FLAG_TRACE_RD		0x00000200	/* Stopped because of read access */
#define _DEBUG_FLAG_TRACE_WR		0x00000400	/* Stopped because of write access */
#define _DEBUG_FLAG_TRACE_MODIFY	0x00000800	/* Stopped because of modified memory */
#define _DEBUG_FLAG_RLC				0x00010000	/* Run-on-Last-Close flag is set */
#define _DEBUG_FLAG_KLC				0x00020000	/* Kill-on-Last-Close flag is set */
#define _DEBUG_FLAG_FORK			0x00040000	/* Child inherits flags (Stop on fork/spawn) */
#define _DEBUG_FLAG_MASK			0x000f0000	/* Flags that can be changed */

enum {
	_DEBUG_WHY_REQUESTED,
	_DEBUG_WHY_SIGNALLED,
	_DEBUG_WHY_FAULTED,
	_DEBUG_WHY_JOBCONTROL,
	_DEBUG_WHY_TERMINATED,
	_DEBUG_WHY_CHILD,
	_DEBUG_WHY_EXEC
};

#define _DEBUG_RUN_CLRSIG			0x00000001	/* Clear pending signal */
#define _DEBUG_RUN_CLRFLT			0x00000002	/* Clear pending fault */
#define _DEBUG_RUN_TRACE			0x00000004	/* Trace mask flags interesting signals */
#define _DEBUG_RUN_HOLD				0x00000008	/* Hold mask flags interesting signals */
#define _DEBUG_RUN_FAULT			0x00000010	/* Fault mask flags interesting faults */
#define _DEBUG_RUN_VADDR			0x00000020	/* Change ip before running */
#define _DEBUG_RUN_STEP				0x00000040	/* Single step only one thread */
#define _DEBUG_RUN_STEP_ALL			0x00000080	/* Single step one thread, other threads run */
#define _DEBUG_RUN_CURTID			0x00000100	/* Change current thread (target thread) */
#define _DEBUG_RUN_ARM				0x00000200	/* Deliver event at point of interest */

typedef struct _debug_process_info {
	pid_t						pid;
	pid_t						parent;
	_Uint32t					flags;
	_Uint32t					umask;
	pid_t						child;
	pid_t						sibling;
	pid_t						pgrp;
	pid_t						sid;
	_Uint64t					base_address;
	_Uint64t					initial_stack;
	uid_t						uid;
	gid_t						gid;
	uid_t						euid;
	gid_t						egid;
	uid_t						suid;
	gid_t						sgid;
	sigset_t					sig_ignore;
	sigset_t					sig_queue;
	sigset_t					sig_pending;
	_Uint32t					num_chancons;
	_Uint32t					num_fdcons;
	_Uint32t					num_threads;
	_Uint32t					num_timers;
	_Uint64t					start_time;		/* Start time in nsec */
	_Uint64t					utime;			/* User running time in nsec */
	_Uint64t					stime;			/* System running time in nsec */
	_Uint64t					cutime;			/* terminated children user time in nsec */
	_Uint64t					cstime;			/* terminated children user time in nsec */
	_Uint8t						priority;		/* process base priority */
	_Uint8t						reserved2[7];
	_Uint8t						extsched[8];
	_Uint64t					pls;			/* Address of process local storage */
	_Uint64t					sigstub;		/* Address of process signal trampoline */
	_Uint64t					canstub;		/* Address of process thread cancellation trampoline */
	_Uint64t					reserved[10];
}							debug_process_t;

typedef struct _debug_thread_info {
	pid_t						pid;
	pthread_t					tid;
	_Uint32t					flags;
	_Uint16t					why;
	_Uint16t					what;
	_Uint64t					ip;
	_Uint64t					sp;
	_Uint64t					stkbase;
	_Uint64t					tls;
	_Uint32t					stksize;
	_Uint32t					tid_flags;
	_Uint8t						priority;
	_Uint8t						real_priority;
	_Uint8t						policy;
	_Uint8t						state;
	_Int16t						syscall;
	_Uint16t					last_cpu;
	_Uint32t					timeout;
	_Int32t						last_chid;
	sigset_t					sig_blocked;
	sigset_t					sig_pending;
	siginfo_t					info;
	union {
		struct {
			pthread_t					tid;
		}							join;
		struct {
			_Int32t						id;
			_Uintptrt					sync;
		}							sync;
		struct {
			_Uint32t					nd;
			pid_t						pid;
			_Int32t						coid;
			_Int32t						chid;
			_Int32t						scoid;
		}							connect;
		struct {
			_Int32t						chid;
		}							channel;
		struct {
			pid_t						pid;
			_Uintptrt					vaddr;
			_Uint32t					flags;
		}							waitpage;
		struct {
			_Uint32t					size;
		}							stack;
		_Uint64t						filler[4];
	}							blocked;
	_Uint64t					start_time;		/* thread start time in nsec */
	_Uint64t					sutime;			/* thread system + user running time in nsec */
	_Uint8t						extsched[8];
	_Uint64t					nsec_since_block;	/*how long thread has been blocked. 0 for STATE_READY or STATE_RUNNING. 
										  in nsec, but ms resolution. */
	_Uint64t					reserved2[4];
}							debug_thread_t;

typedef struct _debug_run {
	_Uint32t					flags;
	pthread_t					tid;
	sigset_t					trace;
	sigset_t					hold;
	fltset_t					fault;
	_Uintptrt					ip;
}							debug_run_t;

typedef struct _debug_break {
	_Uint16t					type;
	_Int16t						size;	/* 1 to 8 for modify, otherwise zero, -1 to remove */
	_Uintptrt					addr;
}							debug_break_t;

enum _debug_break_type {
	_DEBUG_BREAK_EXEC =		0x0001,	/* execution breakpoint */
	_DEBUG_BREAK_RD =		0x0002,	/* read access (fail if not supported) */
	_DEBUG_BREAK_WR =		0x0004,	/* write access (fail if not supported) */
	_DEBUG_BREAK_RW =		0x0006,	/* read or write access (fail if not supported) */
	_DEBUG_BREAK_MODIFY =	0x0008,	/* memory modified */
	_DEBUG_BREAK_RDM =		0x000a,	/* read access if suported otherwise modified */
	_DEBUG_BREAK_WRM =		0x000c,	/* write access if suported otherwise modified */
	_DEBUG_BREAK_RWM =		0x000e,	/* read or write access if suported otherwise modified */
	_DEBUG_BREAK_MASK =		0x000f,	/* mask for matching for replacing/removing breakpoints */
	_DEBUG_BREAK_HW =		0x0010,	/* only use hardware debugging (i.e. no singlestep */
									/* then checking, or writing an invalid instruction to code) */
	_DEBUG_BREAK_SOFT =		0x8000	/* breakpoint inserted/removed during context switch */
};

typedef struct _debug_irq {
	pid_t					pid;
	pthread_t				tid;
	const struct sigevent 	*(*handler)(void *area, int id);
	void					*area;
	unsigned				flags;
	unsigned		 		level;
	unsigned				mask_count;
	int						id;
	unsigned				vector;
	struct sigevent			event;
}							debug_irq_t;

typedef struct _debug_timer {
	timer_t					id;
	unsigned				spare;
	struct _timer_info		info;
}							debug_timer_t;

typedef struct _debug_channel {
	unsigned		chid;
	unsigned char	type;
	unsigned char	zero;
	unsigned short	flags;
	unsigned 		send_queue_depth;
	unsigned 		pulse_queue_depth;
	unsigned 		receive_queue_depth;
	unsigned 		reply_queue_depth;
	unsigned		pad[6]; /* left for future expansion */
}							debug_channel_t;

typedef union _debug_gregs {
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_X86)
	X86_CPU_REGISTERS			x86;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_MIPS)
	MIPS_CPU_REGISTERS			mips;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_PPC)
	PPC_CPU_REGISTERS			ppc;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_ARM)
	ARM_CPU_REGISTERS			arm;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_SH)
	SH_CPU_REGISTERS			sh;
#endif
	_Uint64t						padding[1024];
}							debug_greg_t;

typedef union _debug_fpregs {
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_X86)
	X86_FPU_REGISTERS			x86;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_MIPS)
	MIPS_FPU_REGISTERS			mips;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_PPC)
	PPC_FPU_REGISTERS			ppc;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_ARM)
/*	ARM_FPU_REGISTERS			arm;	*/
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_SH)
	SH_FPU_REGISTERS			sh;	
#endif
	_Uint64t						padding[1024];
}							debug_fpreg_t;

typedef union _debug_altregs {
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_X86)
/*	X86_ALT_REGISTERS			x86;	*/
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_MIPS)
	MIPS_ALT_REGISTERS			mips;	
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_PPC)
	PPC_ALT_REGISTERS			ppc;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_ARM)
/*	ARM_ALT_REGISTERS			arm;	*/
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_SH)
/*	SH_ALT_REGISTERS			sh;	*/
#endif
	_Uint64t						padding[1024];
}							debug_altreg_t;

typedef union _debug_perfregs {
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_X86)
	X86_PERFREGS				x86;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_MIPS)
	MIPS_PERFREGS				mips;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_PPC)
	PPC_PERFREGS				ppc;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_ARM)
	ARM_PERFREGS				arm;
#endif
#if defined(_DEBUG_TARGET_ALL) || defined(_DEBUG_TARGET_SH)
	SH_PERFREGS					sh;
#endif
	_Uint64t						padding[1024];
}							debug_perfreg_t;

#include _NTO_HDR_(_packpop.h)

__END_DECLS

#endif

/* __SRCVERSION("debug.h $Rev: 169879 $"); */
