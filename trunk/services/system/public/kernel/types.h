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

#undef _MEMSIZE_and_memsize_are_different_
#define _MEMSIZE_T_		_Uint64t
#if defined(__PPC__)
/*
 * some of our PPC family compiles with _PADDR_BITS=32 and some with
 * _PADDR_BITS=64 (see proc/<cpu>/cpu.mk)
 * Since the internal 'memsize_t' type is used within procnto AND the
 * libmod_apmxx.a modules, using 'paddr_t' for the derived type 'memsize_t' will
 * result in libmod_apmxx.a being incompatible with some procnto's. To resolve
 * this, we will always declare the internal 'memsize_t' as 64bits for PPC
 * processors. This will result in a slight performance penalty on those PPC
 * processors which would otherwise be compiled with 'memsize_t' of 32 bits.
 * Oh well!   
*/
typedef _MEMSIZE_T_		__attribute__((aligned(8))) memsize_t;
#else	/* defined(__PPC__) */
typedef paddr_t			__attribute__((aligned(8))) memsize_t;
	#if (_PADDR_BITS != 64)	// I wish this could be sizeof(_MEMSIZE_T_) != sizeof(memsize_t)
		#define _MEMSIZE_and_memsize_are_different_
	#endif	/* (_PADDR_BITS == 64) */
#endif	/* defined(__PPC__) */

#include <kernel/proctypes.h>

typedef struct mm_aspace			ADDRESS;
typedef struct breakpt_entry		BREAKPT;
typedef struct channel_entry		CHANNEL;
typedef struct channel_async_entry	CHANNELASYNC;
typedef struct channel_gbl_entry	CHANNELGBL;
typedef struct _client_info			CLIENT;
typedef struct connect_entry		CONNECT;
typedef struct credential_entry		CREDENTIAL;
typedef struct debug_entry			DEBUG;
typedef struct dispatch_entry		DISPATCH;
typedef struct hash_entry			HASH;
typedef struct interrupt_entry		INTERRUPT;
typedef struct interrupt_level		INTRLEVEL;
typedef struct intrevent_entry		INTREVENT;
typedef struct iovec				IOV;
typedef struct limits_entry			LIMITS;
typedef struct memmgr_entry			MEMMGR;
typedef struct net_entry			NET;
typedef struct procmgr_entry		PROCMGR;
typedef struct process_entry		PROCESS;
typedef struct pulse_entry			PULSE;
typedef struct qtime_entry			QTIME;
typedef struct sync_entry			SYNC;
typedef struct session_entry		SESSION;
typedef sigset_t					SIGBITS;
typedef struct sighandler_entry		SIGHANDLER;
typedef struct sigstack_entry		SIGSTACK;
typedef struct sigtable_entry		SIGTABLE;
typedef struct soul_entry			SOUL;
typedef struct syncevent_entry		SYNCEVENT;
typedef struct thread_entry			THREAD;
typedef struct thread_entry			VTHREAD;	// Same as THREAD, but smaller
typedef struct timer_entry			TIMER;
typedef struct vector_entry			VECTOR;
typedef union kerargs				KERARGS;
typedef _Uint32t				MPART_ID;

/* __SRCVERSION("types.h $Rev: 163900 $"); */
