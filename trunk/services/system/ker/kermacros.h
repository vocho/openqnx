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
 * Macros that are private to the kernel itself.
 */

#define PID_MASK		0xfff
#define PINDEX(pid)		((pid) & PID_MASK)

#define MTINDEX(mid)	((mid) >> 16)
#define MCINDEX(mid)	((mid) & 0xffff)

#if defined(WANT_SMP_MACROS) || defined(VARIANT_smp)
	#define KERNCPU					((unsigned)cpunum)
#else
	#define	KERNCPU					0
#endif

/*
 * These macros are for handling scheduler dispatching
 */
#define DISPATCH_THP(dpp, pri)		(THREAD *)((dpp)->ready[pri].head)
#define DISPATCH_LST(dpp, pri)		((dpp)->ready[pri])

#if NUM_PRI <= 8
#define DISPATCH_HIGHEST_PRI(dpp)	(byte_log2[(dpp)->lo])
#define DISPATCH_ISSET(dpp, pri)	((dpp)->lo & (1 << pri))
#define DISPATCH_SET(dpp, thp)		{ (dpp)->lo |= 1 << (thp)->priority; }
#define DISPATCH_CLR(dpp, thp)		{ (dpp)->lo &= ~(1 << (thp)->priority); }

#elif NUM_PRI <= 8 * 8
#define DISPATCH_HI(dpp)			byte_log2[(dpp)->hi]
#define DISPATCH_LO(dpp)			byte_log2[(dpp)->lo[DISPATCH_HI(dpp)]]
#define DISPATCH_HIGHEST_PRI(dpp)	((DISPATCH_HI(dpp)<<3)|DISPATCH_LO(dpp))
#define DISPATCH_ISSET(dpp, pri)	((dpp)->lo[pri >> 3] & (1 << (pri & 7)))
#define DISPATCH_SET(dpp, thp)		{ \
	register unsigned hi = (thp)->priority >> 3; \
	(dpp)->hi |= 1 << hi; \
	(dpp)->lo[hi] |= 1 << ((thp)->priority & 7); \
}
#define DISPATCH_CLR(dpp, thp)		{ \
	register unsigned hi = (thp)->priority >> 3; \
	if(((dpp)->lo[hi] &= ~(1 << ((thp)->priority & 7))) == 0) \
		(dpp)->hi &= ~(1 << hi); \
}

#elif NUM_PRI <= 8 * 8 * 8
#define DISPATCH_HI(dpp)			byte_log2[(dpp)->hi]
#define DISPATCH_MID(dpp)			byte_log2[(dpp)->mid[DISPATCH_HI(dpp)]]
#define DISPATCH_LO(dpp)			byte_log2[(dpp)->lo[DISPATCH_HI(dpp)][DISPATCH_MID(dpp)]]
#define DISPATCH_HIGHEST_PRI(dpp)	((DISPATCH_HI(dpp)<<6)|(DISPATCH_MID(dpp)<<3)|DISPATCH_LO(dpp))
#define DISPATCH_ISSET(dpp, pri)	((dpp)->lo[pri >> 6][(pri >> 3) & 7] & (1 << (pri & 7)))
#define DISPATCH_SET(dpp, thp)		{ \
	register unsigned hi = (thp)->priority >> 6; \
	register unsigned mid = ((thp)->priority >> 3) & 7; \
	(dpp)->hi |= 1 << hi; \
	(dpp)->mid[hi] |= 1 << mid; \
	(dpp)->lo[hi][mid] |= 1 << ((thp)->priority & 7); \
}
#define DISPATCH_CLR(dpp, thp)		{ \
	register unsigned hi = (thp)->priority >> 6; \
	register unsigned mid = ((thp)->priority >> 3) & 7; \
	if(((dpp)->lo[hi][mid] &= ~(1 << ((thp)->priority & 7))) == 0) \
		if(((dpp)->mid[hi] &= ~(1 << mid)) == 0) \
			(dpp)->hi &= ~(1 << hi); \
}

#else
#error NUM_PRI too high
#endif



/*
  These macros allow me to process a signal set which may or may not
  be a scalar.
*/

/* Individual bits */

/* Whole masks */
#define SIGMASK_SET(oset, nset)	((oset)->__bits[0] |= (nset)->__bits[0], (oset)->__bits[1] |= (nset)->__bits[1])
#define SIGMASK_CLR(oset, nset)	((oset)->__bits[0] &=~(nset)->__bits[0], (oset)->__bits[1] &=~(nset)->__bits[1])
#define SIGMASK_CPY(oset, nset)	(*(oset) = *(nset))
#define SIGMASK_ZERO(set) ((set)->__bits[0] = (set)->__bits[1] = 0)
#define SIGMASK_ONES(set) ((set)->__bits[0] = (set)->__bits[1] = ~0)
#define SIGMASK_INDEX(signo) (((signo)-1)>>5)
#define SIGMASK_BIT(signo)   (1 << (((signo)-1) & 0x1f))
#define SIGMASK_NO_KILLSTOP(set)((set)->__bits[0]  &=~(SIGMASK_BIT(SIGKILL)|SIGMASK_BIT(SIGSTOP)))
#define SIGMASK_SPECIAL(set) ((set)->__bits[1] |= 0xff000000)

#define SIG_SET(set, signo)	((set).__bits[SIGMASK_INDEX(signo)] |= SIGMASK_BIT(signo))
#define SIG_CLR(set, signo)	((set).__bits[SIGMASK_INDEX(signo)] &= ~SIGMASK_BIT(signo))
#define SIG_TST(set, signo)	((set).__bits[SIGMASK_INDEX(signo)] & SIGMASK_BIT(signo))

/* Status returns from nano kill calls */
#define SIGSTAT_NOPERMS		0
#define SIGSTAT_IGNORED		1
#define SIGSTAT_QUEUED		2
#define SIGSTAT_NOTQUEUED	4
#define SIGSTAT_ESRCH		8

/*
  These macros convert a realtime signal into a priority for a pulse and back.
*/

#define SIG_TO_PRI(signo)	(_SIGMAX-(int)(signo))
#define PRI_TO_SIG(pri)		(_SIGMAX-(int)(pri))

/* Vectors */
#define VECP(ptr, vec, index)	(((unsigned)((ptr) = (vec)->vector[index]) & 3) ? NULL : (ptr))
#define VECP2(ptr, vec, index)	(((unsigned)((ptr) = (vec)->vector[index]) & 1) ? NULL : VECAND((ptr), ~3))
#define VEC(vec, index)			((vec)->vector[index])
#define VECAND(ptr, bits)		((void *)((bits) & (unsigned)(ptr)))
#define VECOR(ptr, bits)		((void *)((bits) | (unsigned)(ptr)))

/* FPU save areas; low order bits of fpudata is overloaded in SMP case. */
#if PROCESSORS_MAX > 8
	#define FPUDATA_CPUMAX	PROCESSORS_MAX
#else
	#define FPUDATA_CPUMAX	8
#endif
#define FPUDATA_ALIGN		(FPUDATA_CPUMAX<<1)
#define FPUDATA_MASK		(~(FPUDATA_ALIGN-1))
#define FPUDATA_INUSE(ptr)	((uintptr_t)ptr & FPUDATA_BUSY)
#define FPUDATA_CPU(ptr)	((uintptr_t)ptr & FPUDATA_CPUMASK)
#if defined(WANT_SMP_MACROS) || defined(VARIANT_smp)
	#define FPUDATA_BUSY		(FPUDATA_ALIGN>>1)
	#define FPUDATA_CPUMASK		((FPUDATA_ALIGN>>1)-1)
	#define FPUDATA_PTR(ptr)	((void *)((uintptr_t)ptr & FPUDATA_MASK))
#else
	#define FPUDATA_BUSY		0
	#define FPUDATA_CPUMASK		0
	#define FPUDATA_PTR(ptr)	(ptr)
#endif

#define KERCALL_RESTART(act)	{	lock_kernel(); \
									CRASHCHECK(TYPE_MASK(act->type) != TYPE_THREAD); \
									SETKTYPE((act), _TRACE_GETSYSCALL((act)->syscall)); \
									SETKIP(act, KIP(act) - KER_ENTRY_SIZE); }
							

#if !defined(__PPC__) && (defined(WANT_SMP_MACROS) || defined(VARIANT_smp))
//NYI: Kludge - PPC code should be updated to handle need_to_run
#define NEED_PREEMPT(act)	(((act)->priority < queued_event_priority) || need_to_run)
#else
#define NEED_PREEMPT(act)	((act)->priority < queued_event_priority)
#endif

/* Preempt a kernel call if neccessary */
#define KER_PREEMPT(act, err) if(NEED_PREEMPT(act)) {KERCALL_RESTART(act); return(err);}
#define KEREXT_PREEMPT(act)   if(NEED_PREEMPT(act)) {KERCALL_RESTART(act); return;}
/* Preempt a specret if neccessary; does not look at need_to_run because otherwise we'd never exit */
#define SPECRET_PREEMPT(act)	if((act)->priority < queued_event_priority) { lock_kernel(); unspecret_kernel(); return; }

/* Check for an immediate timeout */
#define IMTO(thp, state) (((thp)->timeout_flags & (1 << (state)))  &&  ((thp)->timeout_flags & _NTO_TIMEOUT_IMMEDIATE))

/* Check for a pending thread cancel */
#define PENDCAN(flags) (((flags) & (PTHREAD_CANCEL_DISABLE|PTHREAD_CANCEL_PENDING)) == PTHREAD_CANCEL_PENDING)

#define STATE_CANCEL_BITS	((1<<STATE_SEND) |\
							 (1<<STATE_RECEIVE) |\
							 (1<<STATE_REPLY) |\
							 (1<<STATE_SIGSUSPEND) |\
							 (1<<STATE_SIGWAITINFO) |\
							 (1<<STATE_NANOSLEEP) |\
							 (1<<STATE_CONDVAR) |\
							 (1<<STATE_JOIN) |\
							 (1<<STATE_INTR) |\
							 (1<<STATE_SEM))
#define STATE_CANCELABLE(thp)	(((1 << (thp)->state) & STATE_CANCEL_BITS) && \
								KTYPE(thp) != __KER_MSG_SENDVNC)

#define STATE_LAZY_RESCHED_BITS	((1<<STATE_REPLY) | (1<<STATE_WAITPAGE) | (1<<STATE_STACK))
#define STATE_LAZY_RESCHED(thp)	((1 << (thp)->state) & STATE_LAZY_RESCHED_BITS)

/* Macros for playing with the owner field in a mutex sync_t */
#define SYNC_OWNER_BITS(pid,tid)	((((pid) << 16) | (tid) + 1) & ~_NTO_SYNC_WAITING)
#define SYNC_OWNER(thp)	SYNC_OWNER_BITS((thp)->process->pid, (thp)->tid)
#define SYNC_PINDEX(owner)	PINDEX(((owner) & ~_NTO_SYNC_WAITING) >> 16)
#define SYNC_TID(owner)	(((owner) & 0xffff) - 1)

/* Flag and init macro for soul lists */
#define INITSOUL(a,b,c,d,e)		= { 0, a, 0, 0, b, 0, c, c, 0, e, d }

/* Access various special registers */
#define KTYPE(thp)			REGTYPE(&(thp)->reg)
#define KSTATUS(thp)		REGSTATUS(&(thp)->reg)
#define KIP(thp)			REGIP(&(thp)->reg)
#define KSP(thp)			REGSP(&(thp)->reg)

#define SETKTYPE(thp, v)	SETREGTYPE(&(thp)->reg, v)
#define SETKSTATUS(thp,v)	SETREGSTATUS(&(thp)->reg, v)
#define SETKSP(thp,v)		SETREGSP(&(thp)->reg, v)
#define SETKIP(thp,v)		SETREGIP(&(thp)->reg, v)
#ifndef SETKIP_FUNC
    #define SETKIP_FUNC(thp,v)		SETKIP(thp, v)
#endif

/* Manipulate the thread's stack */
#ifdef STACK_GROWS_UP
	#define STACK_INIT(bottom, size)	(bottom)
	#define STACK_ALLOC(res, new_sp, curr_sp, size)	\
				(res) = (void *)(curr_sp)),	\
				((new_sp) = ((curr_sp) + (size) + (STACK_ALIGNMENT-1)) & ~(STACK_ALIGNMENT-1)
#else
	#define STACK_INIT(bottom, size)	((bottom) + (size))
	#define STACK_ALLOC(res, new_sp, curr_sp, size)	\
				((new_sp) = ((curr_sp) - (size)) & ~(STACK_ALIGNMENT-1),	\
				(res) = (void *)(new_sp))
#endif

/* Used to force an EOK status on return from a kernel routine */
#define ENOERROR	(-1L)

/* Convert a hook vector number to an interrupt level */
#define HOOK_TO_LEVEL(vector)	(-(((vector)+2)&~_NTO_INTR_CLASS_SYNTHETIC))

/*
 * Manipulate inkernel state
 */
#ifndef INKERNEL_BITS_SHIFT
    #define INKERNEL_BITS_SHIFT	4
#endif
#define INKERNEL_INTRMASK	((1 << INKERNEL_BITS_SHIFT) - 1)
#define INKERNEL_EXIT		(0x0001 << INKERNEL_BITS_SHIFT)
#define INKERNEL_SPECRET	(0x0002 << INKERNEL_BITS_SHIFT)
#define INKERNEL_LOCK		(0x0004 << INKERNEL_BITS_SHIFT)
#define INKERNEL_NOW		(0x0008 << INKERNEL_BITS_SHIFT)

#if defined(COMPILING_MODULE)
	// We can't use the following macros when compiling a module - have
	// to invoke the appropriate functions that live in the kernel proper
	// to get the correct versions for the variant that we're running
	// on. Define the macros such that people will get an error message
	// if they try to use them.
	#undef am_inkernel
	#undef bitset_inkernel
	#undef bitclr_inkernel
	#define am_inkernel() use_KerextAmInKernel_when_in_a_module
	#define bitset_inkernel() use_KerextLock_when_in_a_module
	#define bitclr_inkernel() use_KerextUnlock_when_in_a_module
#endif	

#if defined(VARIANT_smp) && !(defined(am_inkernel) && defined(bitset_inkernel) && defined(bitclr_inkernel))
	#error am_inkernel(), bitset_inkernel(), bitclr_inkernel()
	#error need overrides when building SMP system
#endif
#ifdef get_inkernel
	#define HAVE_INKERNEL_STORAGE 1
#else
	#define get_inkernel()		inkernel
	#define set_inkernel(val)	(inkernel=(val))
#endif
#ifndef am_inkernel
	#define am_inkernel()		(get_inkernel() != 0)
#endif
#define ASM_VOLATILIZER	asm volatile ("":::"memory")
#ifndef ASM_VOLATILIZER
#define ASM_VOLATILIZER
#endif
#ifndef bitset_inkernel
	#define bitset_inkernel(bits)	(set_inkernel(get_inkernel() | (bits)))
#endif
#ifndef bitclr_inkernel
	#define bitclr_inkernel(bits)	(set_inkernel(get_inkernel() & ~(bits)))
#endif

#ifndef atomic_order
	// atomic_order() makes sure that the compiler doesn't re-order things
	// across the given statement. Otherwise we might end up modifying kernel 
	// state before we've actually locked the kernel.
	#if (defined(__GNUC__) || defined(__ICC))
		static __inline__ void __attribute__((__unused__)) atomic_boundry() { asm volatile( "" ::: "memory"); }
		#define atomic_order(s)	(atomic_boundry(), (s), atomic_boundry()) 
	#elif defined(__WATCOMC__)
		#define atomic_order(s)	(s)
	#else
		#error atomic_order not defined for compiler
	#endif
#endif
#define lock_kernel()			(atomic_order(bitset_inkernel(INKERNEL_LOCK)))
#define unlock_kernel()			atomic_order(bitclr_inkernel(INKERNEL_LOCK))
#define specret_kernel()		atomic_order(bitset_inkernel(INKERNEL_SPECRET))
#define unspecret_kernel()		(atomic_order(bitclr_inkernel(INKERNEL_SPECRET)),inspecret=0)

#ifndef CONDITION_CYCLES
	#define CONDITION_CYCLES(c)	((unsigned long)c)
#endif


/* Check passed in pointers to make sure that they're an OK address */

#ifdef CPU_RD_SYSADDR_OK
	#define RD_VERIFY_PTR(thp, p, size)	if(!WITHIN_BOUNDRY((uintptr_t)(p),(uintptr_t)(p)+(size),(thp)->process->boundry_addr) && !CPU_RD_SYSADDR_OK((thp), (p), (size))) return EFAULT;
#else		
	#define RD_VERIFY_PTR(thp, p, size)	if(!WITHIN_BOUNDRY((uintptr_t)(p),(uintptr_t)(p)+(size),(thp)->process->boundry_addr)) return EFAULT;
#endif		
#define WR_VERIFY_PTR(thp, p, size)	if(!WITHIN_BOUNDRY((uintptr_t)(p),(uintptr_t)(p)+(size),(thp)->process->boundry_addr)) return EFAULT;
#define WR_PROBE_PTR(thp, p, size)	(((uintptr_t)(p) + (size) >= (uintptr_t)(p)) && WITHIN_BOUNDRY((uintptr_t)(p),(uintptr_t)(p)+(size),(thp)->process->boundry_addr))

#define RD_PROBE_INT(thp,p,num)	((num)==1 ? rd_probe_1(p) : rd_probe_num((p),(num)))
#define WR_PROBE_INT(thp,p,num)	((num)==1 ? wr_probe_1(p) : wr_probe_num((p),(num)))
#ifndef	WR_PROBE_OPT
#define	WR_PROBE_OPT(thp,p,num)
#endif

#define _NTO_BILLION			1000000000UL
#define _NTO_MILLION			1000000UL
#define _TOPBIT					(~0u ^ (~0u >> 1))
#define _FORCE_SET_ERROR		_TOPBIT
#define _FORCE_KILL_SELF		(_TOPBIT>>1)
#define _FORCE_NO_UNBLOCK		(_TOPBIT>>2)
// If another _FORCE_* flag is added, be sure to add it to the _FORCE_BITS
// definition below and make sure that nano_sched.c is recompiled so that
// we properly mask all these bits off before passing the errno to kererr()		
#define _FORCE_BITS				(_FORCE_SET_ERROR|_FORCE_KILL_SELF|_FORCE_NO_UNBLOCK)

//
// Configuration constants
//
#define _NTO_TICKSIZE_MIN	10000		// Smallest ticksize allowed (10 usec for now)
#define _NTO_TICKSIZE_MAX	(INT_MAX/_NTO_RR_INTERVAL_MUL) // Largest ticksize allowed
#define _NTO_SLOW_MHZ		40			// A machine less than this number of mhz is concidered slow
#define _NTO_TICKSIZE_FAST	1000000		// Ticksize in ns to default to for a fast machine (less than 1ms for posix)
#define _NTO_TICKSIZE_SLOW	10000000	// Ticksize in ns to default to for a slow machine
#define _NTO_RR_INTERVAL_MUL_DEFAULT 4  // Default ValueMultiplier of ticksize to round robin interval
#define _NTO_RR_INTERVAL_MUL rr_interval_mul // Multiplier of ticksize to round robin interval

#define THREAD_NAME_FIXED_SIZE	24		// Thread name pool object size, not max thread name

#define VECTOR_MAX			(0xffff-1)

//
// Max number of people who can attach to one particular interrupt level
//
#define INTR_LEVEL_ATTACH_MAX	32

//
// Values used within a pulse entry
//
	// Maximum count for compressing pulses
#define _PULSE_COUNT_MAX			((2U<<(sizeof(((struct pulse_entry *)0)->count)*8-1)) - 1U)

//
// Bits used by xfermsg
//
#define XFER_SRC_FAULT	0x01
#define XFER_DST_FAULT	0x02

#define XFER_SRC_CHECK	0x01
#define XFER_DST_CHECK	0x02

//
// Bits used by internal_flags
//
#define _NTO_ITF_MSG_DELIVERY	0x01
#define _NTO_ITF_MSG_FORCE_RDY	0x02
#define _NTO_ITF_NET_UNBLOCK	0x04 /* for vthread only */
#define _NTO_ITF_SSTEP_SUSPEND	0x08 
#define _NTO_ITF_SPECRET_PENDING 0x10 
    /* SPECRET_PENDING: some other thread needs this thread's data for specialret().
     * It is set for a sending thread when a message or pulse is sent that causes
     * RCVINFO or SHORT_MSG to be set in the receiver's flags.  Note that it does not
     * get used when SHORT_MSG is set in a reply, as in that case the thread that is
     * receiving the reply does not depend on the thread sending the reply.
     */
//#define unused		0x20
#define _NTO_ITF_UNBLOCK_QUEUED	0x40
#define _NTO_ITF_RCVPULSE		0x80

//
//	These flags are set asynchronously by the kernel.
//
#define _NTO_ATF_TIMESLICE		0x00000001
#define _NTO_ATF_FPUSAVE_ALLOC	0x00000002
#define _NTO_ATF_SMP_RESCHED	0x00000004
#define _NTO_ATF_SMP_EXCEPTION	0x00000008
#define _NTO_ATF_WAIT_FOR_KER	0x00000010	
#define _NTO_ATF_WATCHPOINT		0x00000020
#define _NTO_ATF_REGCTX_ALLOC	0x40000000
#define _NTO_ATF_FORCED_KERNEL	0x80000000

//
// used to set/read xfer handler
//
#if defined(SMP_MSGOPT)
#define SET_XFER_HANDLER(handler)	xfer_handlers[KERNCPU] = handler
#define GET_XFER_HANDLER()			(xfer_handlers[KERNCPU]) 
#else
#define SET_XFER_HANDLER(handler)	xfer_handlers[0] = handler
#define GET_XFER_HANDLER()			(xfer_handlers[0])
#endif

//
// flags for sync_mutex_lock
//
#define _MUTEXLOCK_FLAGS_NOCEILING		0x1

//
// flags for pulses/signals
#define _PULSE_PRIO_DPP_SHIFT	8
#define _PULSE_PRIO_DPP_MASK	0x00000f00
#define _PULSE_PRIO_DPP_VALID	0x00001000
#define _PULSE_PRIO_CRITICAL	0x00002000
#define _PULSE_PRIO_BOOST       0x00004000
#define _PULSE_PRIO_REPLACE     0x10000000
#define _PULSE_PRIO_SIGNAL      0x20000000
#define _PULSE_PRIO_VTID        0x40000000
#define _PULSE_PRIO_HEAD		0x80000000

#define _PULSE_PRIO_PUBLIC_MASK 0xffff

// macros for Round Robin
#define RR_MAXTICKS			(_NTO_RR_INTERVAL_MUL*2)
#define RR_FULLTICK			(2)
#define RR_PREEMPT_TICK 	(1)
#define IS_SCHED_RR(thp)	( ((thp)->policy == SCHED_RR) || ((thp)->policy == SCHED_OTHER) )
#define RR_ADD_FULLTICK(thp)	if (IS_SCHED_RR((thp))) { \
									(thp)->schedinfo.rr_ticks = ((thp)->schedinfo.rr_ticks <= (RR_MAXTICKS-RR_FULLTICK)) \
											? (thp)->schedinfo.rr_ticks+RR_FULLTICK : RR_MAXTICKS; }
 
#define RR_ADD_PREEMPT_TICK(thp) if (IS_SCHED_RR((thp))) { \
									(thp)->schedinfo.rr_ticks = ((thp)->schedinfo.rr_ticks <= (RR_MAXTICKS-RR_PREEMPT_TICK)) \
											? (thp)->schedinfo.rr_ticks+RR_PREEMPT_TICK : RR_MAXTICKS; }

#define RR_RESET_TICK(thp)	if (IS_SCHED_RR((thp))) { \
								(thp)->schedinfo.rr_ticks = 0; }
#define RR_GET_TICKS(thp)	((thp)->schedinfo.rr_ticks)

// macros for Sporadic
#define IS_SCHED_SS(thp)	((thp)->policy == SCHED_SPORADIC)

/**
 Note the running time for this thread.  This may cause a call to adjust priority
 to drop the priority of the thread, so only call this when the actives are stable
 and the thp is on the appropriate priority list.  This call will only adjust the
 priority of the specified thread downward.
**/
#define SS_STOP_RUNNING(thp, preempted) if(IS_SCHED_SS((thp))) { sched_ss_block((thp), (preempted)); }

/** 
 Note the time that this thread was first made read/running.  This is used for
 later scheduling the replenishment operations.  
**/
#define SS_MARK_ACTIVATION(thp) 	if(IS_SCHED_SS((thp))) { sched_ss_queue((thp)); } 

/**
 Since the scheduler can be entered from many points, this macro allows us to do
 a check to see if the SS thread specified has expired.  This call could result
 in a drop in the priority of this thread.  This drop should only execute any
 code when the caller is coming through from the interrupt handler since that is
 the only place where the curr_budget will be zero'ed out at high priority.
**/
#define SS_CHECK_EXPIRY(thp)		if(IS_SCHED_SS((thp)) && \
										(thp)->schedinfo.ss_info->curr_budget == 0 && \
										(thp)->schedinfo.ss_info->org_priority == 0) { \
											sched_ss_update((thp), (thp)->schedinfo.ss_info, &((thp)->schedinfo.ss_info->replenishment), 1); \
									}


//For identifying which external scheduler is loaded. assign to externs.h: scheduler_type 
#define SCHEDULER_TYPE_DEFAULT	0
#define SCHEDULER_TYPE_APS 		1
		

//macros for APS scheduling
//
/* values for thread_entry.sched_flags */
#define AP_SCHED_RUNNING_CRIT	0x00000001 /*is our thread critical now? */
#define AP_SCHED_PERM_CRIT 		0x00000002 /*thread is always running critical. means AP_SCHED_RUNNING_CRIT is never cleared */
#define AP_SCHED_BILL_AS_CRIT	0x00000004 /*not all critcal time is billed as critical. Bit on means bill to crit bugdet */ 

#define AP_CLEAR_CRIT(thp) (\
	(thp)->sched_flags &= ~AP_SCHED_BILL_AS_CRIT, \
	(thp)->sched_flags &= ((thp)->sched_flags & AP_SCHED_PERM_CRIT) ? ~0 : ~AP_SCHED_RUNNING_CRIT \
) 

#define AP_INHERIT_CRIT(to_thp,from_thp) ((to_thp)->sched_flags =\
		(~(AP_SCHED_RUNNING_CRIT|AP_SCHED_BILL_AS_CRIT)&(to_thp)->sched_flags )\
		| ((from_thp)->sched_flags & (AP_SCHED_RUNNING_CRIT|AP_SCHED_BILL_AS_CRIT) )\
	)\


#define AP_MARK_THREAD_CRITICAL(thp) ((thp)->sched_flags |= AP_SCHED_RUNNING_CRIT) 	

/* The following macro and flag is to allow intrevent_add to deterimine if it's being called from an IO interrupt.
 * (If * intrevent_add is called from IO, it will set the SIGEV_FLAG_CRITICAL bit to make the final receiving thread run
 * critical.) Use the following macro to pass the thp parameter to intrevent_add. ex:
 * intrevent_add(evp, AP_INTREVENT_FROM_IO(thp), 1); 
 */
#define AP_INTREVENT_FROM_IO_FLAG 1 
#define AP_INTREVENT_FROM_IO(thp) ( (THREAD*)((uintptr_t)(thp) | AP_INTREVENT_FROM_IO_FLAG) )
	
	
// end APS macros 


// flag options for nano_pulse.c:: pulse_deliver() 
#define PULSE_DELIVER_APS_CRITICAL_FLAG 0x00000001

//flag options for nano_thread.c :: thread_create() 
#define THREAD_CREATE_BLOCK_FLAG			0x00000001
#define THREAD_CREATE_APS_CRITICAL_FLAG	0x00000002

//flag options for signal_kill_process/signal_kill_thread/signal_kill_group
#define SIGNAL_KILL_APS_CRITICAL_FLAG	0x00000001
#define SIGNAL_KILL_TERMER_INHERIT_FLAG	0x00000002

#define SNAP_TIME_INLINE(tsp,incl_tod) \
{ \
	int64_t		adjust; \
	extern volatile uint64_t *nssptr; \
	do { \
		(tsp) = qtimeptr->nsec; \
		if((tsp) == -1) { \
			snap_time_wakeup(); \
		} \
		adjust = qtimeptr->nsec_tod_adjust; \
		/* Try again if the time has changed under our feet. */ \
	} while((tsp) != *nssptr); \
	if((incl_tod)) { \
		(tsp) += adjust; \
	} \
}

// allow cpu-specific override for manipulating intr_slock
#ifndef CPU_SLOCK_INTR_LOCK
#	define	CPU_SLOCK_INTR_LOCK(l)		INTR_LOCK(l)
#endif
#ifndef	CPU_SLOCK_INTR_UNLOCK
#	define	CPU_SLOCK_INTR_UNLOCK(l)	INTR_UNLOCK(l)
#endif
#ifndef CPU_SLOCK_LOCK
#	define	CPU_SLOCK_LOCK(l)			SPINLOCK(l)
#endif
#ifndef	CPU_SLOCK_UNLOCK
#	define	CPU_SLOCK_UNLOCK(l)			SPINUNLOCK(l)
#endif

//Macro definitions to deal with pril objects on lists that don't have to
//be ordered - should figure out a better way of handling them later...
#define LINKPRIL_REM(_object) LINK2_REM((LINK3_NODE *)(_object))
#define LINKPRIL_BEG(_queue, _object, _type) LINK2_BEG(*(LINK3_NODE **)&(_queue), (LINK3_NODE *)(_object), _type)

/* __SRCVERSION("kermacros.h $Rev: 206796 $"); */
