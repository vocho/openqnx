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
 *  trace.h     Trace data structures and definitions
 *

 */

#ifndef __KERTRACE_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif
#ifndef _STDDEF_H_INCLUDED
 #include <stddef.h>
#endif
#ifndef __TRACE_H_INCLUDED
 #include <sys/trace.h>
#endif

typedef struct {
	uint32_t time_off;
	uint32_t data_emitted;	//Data has been emitted
	uint32_t di;			//Byte offset filled
	union {
		uint32_t idata[2];
		char	 cdata[8];
	} data;
} trace_state_t;

typedef struct ehandler_data {
	int                    (*handler)(event_data_t*);
	event_data_t*          area;
	THREAD*                thp;
	PROCESS*               process;
	struct ehandler_data** location;
  struct cpu_intrsave    cpu;
} ehandler_data_t;

// Prototype of event emitting function
int em_event(tracebuf_t*);


// Getting/setting events/classes
#define _TRACE_ROUND_UP(n)          (((n)&3)?(((n)>>2)+1):((n)>>2))
#define _TRACE_SET_STRUCT(c, st)    (c&=~(0x3<<30),c|=(st))
#define _TRACE_MAKE_CODE(c,f,cl,e)  (((c)<<24)|(f)|(cl)|(e))


// platform dependent pointer argument access and ClockCycles() ret.
#ifdef __BIGENDIAN__
 #define _TRACE_CLOCK_MSB           (0^_TRACE_CLOCK_SWAP)
 #define _TRACE_CLOCK_LSB           (1^_TRACE_CLOCK_SWAP)
#else
 #define _TRACE_CLOCK_MSB           (1^_TRACE_CLOCK_SWAP)
 #define _TRACE_CLOCK_LSB           (0^_TRACE_CLOCK_SWAP)
#endif
#if defined(__X86__)
 #define _TRACE_ARGPTR(thp)         ((KSP(thp))+4)
 #define _TRACE_CLOCK_SWAP          (0)
 #define _TRACE_SAVE_REGS(t,c)
#elif defined(__PPC__)
 #define _TRACE_ARGPTR(thp)         (&(thp)->reg.gpr[3])
 #define _TRACE_CLOCK_SWAP          (0)
 #define _TRACE_SAVE_REGS(t,c)      ((c)->gpr2=(t)->reg.gpr[2],(c)->gpr13=(t)->reg.gpr[13])
#elif defined(__MIPS__)
 #define _TRACE_ARGPTR(thp)         (&(thp)->reg.regs[MIPS_BAREG(MIPS_REG_A0)])
 #define _TRACE_CLOCK_SWAP          (1)
 #define _TRACE_SAVE_REGS(t,c)      ((c)->gp=*(uint64_t*)&(t)->reg.regs[MIPS_BAREG(MIPS_REG_GP)])
#elif defined(__ARM__)
 #define _TRACE_ARGPTR(thp)         ((thp)->reg.gpr)
 #define _TRACE_CLOCK_SWAP          (0)
 #define _TRACE_SAVE_REGS(t,c)
#elif defined(__SH__)
 #define _TRACE_ARGPTR(thp)         (&(thp)->reg.gr[4])
 #define _TRACE_CLOCK_SWAP          (1)
 #define _TRACE_SAVE_REGS(t,c)
#else
 #error instrumentation not supported
#endif

#define _TRACEDATANUM               (2)
#define _TRACE_MAX_STR_SIZE         (128)
#define _TRACE_MAX_SMP_CPU_NUM      (64)
#define _TRACE_MAX_EV_HANDLER_NUM   (128)

#define _TRACE_ENTER_CALL           (0x00000001<<0)
#define _TRACE_EXIT_CALL            (0x00000001<<1)
#define _TRACE_CALL_ARG_WIDE        (0x00000001<<2)
#define _TRACE_CALL_RET_WIDE        (0x00000001<<3)

#define _TRACE_ENTER_SYSTEM         (0x00000001<<0)
#define _TRACE_SYSTEM_ARG_WIDE      (0x00000001<<1)

#define _TRACE_ENTER_COMM           (0x00000001<<0)
#define _TRACE_COMM_ARG_WIDE        (0x00000001<<1)

#define _TRACE_THREAD_ARG_WIDE      (0x80000000)

#define _TRACE_ENTER_INT            (0x00000001<<0)
#define _TRACE_EXIT_INT             (0x00000001<<1)

#define _TRACE_ENTER_USER           (0x00000001<<0)

#define _TRACE_SEC(a)        ((uint32_t)(((uint64_t)(a))/((uint64_t)1000000000)))
#define _TRACE_NSEC(a)       ((uint32_t)(((uint64_t)(a))%((uint64_t)1000000000)))
#define _TRACE_GET_LOCK()    (get_inkernel()&INKERNEL_LOCK)
#define _TRACE_CHK_UNLOCK(l) (!l&&get_inkernel()&INKERNEL_LOCK)

#define _TRACE_INT_LEV(l)    (((uint32_t)(l) - (uint32_t)interrupt_level) / \
                             (sizeof(struct interrupt_level)))
#define _TRACE_INT_NUM(l)    (((struct interrupt_level *) (l))->info->vector_base + \
                             _TRACE_INT_LEV(l) - ((struct interrupt_level *) (l))->level_base)

#if defined(VARIANT_instr)

#if !defined(NDEBUG) && !defined(__WATCOMC__)
#define _TRACE_PRINTF(code,x,...)	add_trace_string(_TRACE_MAKE_CODE( RUNCPU, _TRACE_STRUCT_S, _TRACE_USER_C, (code)), (x), __VA_ARGS__)
#endif

// enter/exit states/arguments control macros
#define _TRACE_GETSYSCALL(c)        ((c)&0xff)
#define _TRACE_NOARGS(thp)          (thp)->syscall|=(0x1<<10)
#define _TRACE_GETARGSFLAG(thp)     (!((thp)->syscall&(0x1<<10)))
#define _TRACE_COMM_IPC_EXIT        (0x4000)
#define _TRACE_COMM_CALL_EXIT       (0x8000)

#define _TRACE_DER_PTR(p,m)  ((p)?(p)->m:NULL)

#define _TRACE_PIDTID(a,p,t) (!(trace_masks.p)||(trace_masks.p)== \
                             (uint32_t)_TRACE_DER_PTR((a)->process,pid)&& \
                             (!(trace_masks.t)||(trace_masks.t)== \
                             ((uint32_t)(a)->tid+1)))

#define _TRACE_THPTID(a,n)   _TRACE_PIDTID(a,thread_pid[n],thread_tid[n])
#define _TRACE_VTHPTID(a,n)  _TRACE_PIDTID(a,vthread_pid[n],vthread_tid[n])
#define _TRACE_IPCPTID(a,e)  _TRACE_PIDTID(a,comm_pid[e],comm_tid[e])

// Control/process/thread state macro definitions


// For some macros, the argument can be either a thread or vthread.
// Do the right thing.
#define _TRACE_TH_EMIT_STATE(t,s)		  \
if (TYPE_MASK(t->type) == TYPE_VTHREAD) { \
	if (trace_masks.vth_mask&_NTO_TRACE_VTH ## s)   vth_em_st(t, STATE_ ## s); \
} else { \
	if (trace_masks.th_mask&_NTO_TRACE_TH ## s)     th_em_st(t, STATE_ ## s, RUNCPU); \
}

#define _TRACE_TH_EMIT_CREATE(t)		  \
if (TYPE_MASK(t->type) == TYPE_VTHREAD) { \
	if (trace_masks.vth_mask&_NTO_TRACE_VTHCREATE)  vth_em_cd(t, _TRACE_THREAD_CREATE, _TRACE_PR_TH_CREATE_VT); \
} else { \
	if (trace_masks.th_mask&_NTO_TRACE_THCREATE)    th_em_cd(t, _TRACE_THREAD_CREATE, _TRACE_PR_TH_CREATE_T); \
}

#define _TRACE_TH_EMIT_DESTROY(t) \
if (TYPE_MASK(t->type) == TYPE_VTHREAD) { \
	if (trace_masks.vth_mask&_NTO_TRACE_VTHDESTROY) vth_em_cd(t, _TRACE_THREAD_DESTROY, _TRACE_PR_TH_DESTROY_VT); \
} else { \
	if (trace_masks.th_mask&_NTO_TRACE_THDESTROY)   th_em_cd(t, _TRACE_THREAD_DESTROY, _TRACE_PR_TH_DESTROY_T); \
}

#define _TRACE_TH_EMIT_STATE_ONCPU(t,s,c) \
if (trace_masks.th_mask&(0x00000001<<(s)))      th_em_st(t, s, c);
#define _TRACE_TH_EMIT_ANY_STATE(t,s) \
if (trace_masks.th_mask&(0x00000001<<(s)))      th_em_st(t, s, RUNCPU);
#define _TRACE_TH_EMIT_NAME(t) \
if (trace_masks.pr_mask&_NTO_TRACE_PROCTHREAD_NAME)  th_em_name(t);
#define _TRACE_PR_EMIT_CREATE(p) \
if (trace_masks.pr_mask&_NTO_TRACE_PROCCREATE)  pr_em_create(p, (p)->pid);
#define _TRACE_PR_EMIT_DESTROY(p,id) \
if (trace_masks.pr_mask&_NTO_TRACE_PROCDESTROY) pr_em_destroy(p, id);
#define _TRACE_PR_EMIT_CREATE_NAME(p) \
if (trace_masks.pr_mask&_NTO_TRACE_PROCCREATE_NAME)  pr_em_create_name(p);
#define _TRACE_PR_EMIT_DESTROY_NAME(p) \
if (trace_masks.pr_mask&_NTO_TRACE_PROCDESTROY_NAME) pr_em_destroy_name(p);
#define _TRACE_EMIT_CONTROL_TIME(m,l) \
if (trace_masks.control_mask&_NTO_TRACE_CONTROLTIME) time_em(m, l);
#define _TRACE_COMM_EMIT_SMSG(a,c,i) \
if (trace_masks.comm_mask[_NTO_TRACE_COMM_SMSG]&_TRACE_ENTER_COMM) comm_em(a, c, i, _NTO_TRACE_COMM_SMSG);
#define _TRACE_COMM_EMIT_SPULSE(a,c,s,p) \
if (trace_masks.comm_mask[_NTO_TRACE_COMM_SPULSE]&_TRACE_ENTER_COMM) comm_em(a, c, s, _NTO_TRACE_COMM_SPULSE);
#define _TRACE_COMM_EMIT_SPULSE_EV(c,p,s,e) \
if(trace_masks.comm_mask[e]&_TRACE_ENTER_COMM) comm_exe_em((c), (uint32_t) (s), (e));
#define _TRACE_COMM_EMIT_REPLY(a,c,i) \
if (trace_masks.comm_mask[_NTO_TRACE_COMM_REPLY]&_TRACE_ENTER_COMM) comm_em(a, c, i, _NTO_TRACE_COMM_REPLY);
#define _TRACE_COMM_EMIT_ERROR(a,c,i) \
if (trace_masks.comm_mask[_NTO_TRACE_COMM_REPLY]&_TRACE_ENTER_COMM) comm_em(a, c, i, _NTO_TRACE_COMM_ERROR);
#define _TRACE_COMM_EMIT_SPULSE_EXE(c,s,p) _TRACE_COMM_EMIT_SPULSE_EV(c, p, s, _NTO_TRACE_COMM_SPULSE_EXE)
#define _TRACE_COMM_EMIT_SPULSE_DIS(c,s,p) _TRACE_COMM_EMIT_SPULSE_EV(c, p, s, _NTO_TRACE_COMM_SPULSE_DIS)
#define _TRACE_COMM_EMIT_SPULSE_DEA(c,s,p) _TRACE_COMM_EMIT_SPULSE_EV(c, p, s, _NTO_TRACE_COMM_SPULSE_DEA)
#define _TRACE_COMM_EMIT_SPULSE_UN(c,s,p)  _TRACE_COMM_EMIT_SPULSE_EV(c, p, s, _NTO_TRACE_COMM_SPULSE_UN)
#define _TRACE_COMM_EMIT_SPULSE_QUN(s,p) _TRACE_COMM_EMIT_SPULSE_EV(NULL, p, s, _NTO_TRACE_COMM_SPULSE_QUN)
#define _TRACE_COMM_IPC_RET(a)                  (a->syscall|=_TRACE_COMM_IPC_EXIT)
#define _TRACE_DESTROY_EH(p)                    destroy_eh(p)
#define _TRACE_COMM_EMIT_SIGNAL(t,s) \
if (trace_masks.comm_mask[_NTO_TRACE_COMM_SIGNAL]&_TRACE_ENTER_COMM) comm_em_signal(t, s)

#define _TRACE_SYS_APS_NAME(id,namel)\
if (trace_masks.system_mask[_NTO_TRACE_SYS_APS_NAME]&_TRACE_ENTER_SYSTEM) aps_name_em(id, name);
#define _TRACE_SYS_APS_BUDGETS(id,percent,critical)\
if (trace_masks.system_mask[_NTO_TRACE_SYS_APS_BUDGETS]&_TRACE_ENTER_SYSTEM) aps_budgets_em(id, percent, critical);
#define _TRACE_SYS_APS_BANKRUPTCY(id,pid,tid) \
if (trace_masks.system_mask[_NTO_TRACE_SYS_APS_BNKR]&_TRACE_ENTER_SYSTEM) aps_bankruptcy_em(id, pid, tid);

#define _TRACE_SYS_EMIT_ADDRESS(thp,vaddr)\
if (trace_masks.system_mask[_NTO_TRACE_SYS_ADDRESS]&_TRACE_ENTER_SYSTEM) trace_emit_address((thp),(vaddr));

#else

#define _TRACE_TH_EMIT_CREATE(t)
#define _TRACE_TH_EMIT_DESTROY(t)
#define _TRACE_TH_EMIT_NAME(t)
#define _TRACE_TH_EMIT_STATE(t,s)
#define _TRACE_TH_EMIT_ANY_STATE(t,s)
#define _TRACE_PR_EMIT_CREATE(p)
#define _TRACE_PR_EMIT_CREATE_NAME(p)
#define _TRACE_PR_EMIT_DESTROY(p,id)
#define _TRACE_PR_EMIT_DESTROY_NAME(p)
#define _TRACE_DESTROY_EH(p)
#define _TRACE_EMIT_CONTROL_TIME(m,l)
#define _TRACE_COMM_EMIT_SMSG(a,c,i)
#define _TRACE_COMM_EMIT_SPULSE(a,c,s,p)
#define _TRACE_COMM_EMIT_SPULSE_EXE(c,s,p)
#define _TRACE_COMM_EMIT_SPULSE_DIS(c,s,p)
#define _TRACE_COMM_EMIT_SPULSE_DEA(c,s,p)
#define _TRACE_COMM_EMIT_SPULSE_UN(c,s,p)
#define _TRACE_COMM_EMIT_SPULSE_QUN(s,p)
#define _TRACE_COMM_EMIT_SIGNAL(t,s)
#define _TRACE_COMM_EMIT_REPLY(a,c,i)
#define _TRACE_COMM_EMIT_ERROR(a,c,i)
#define _TRACE_COMM_IPC_RET(a)
#define _TRACE_NOARGS(c)
#define _TRACE_GETSYSCALL(c)            (c)
#define _TRACE_SYS_APS_NAME(id,name)
#define _TRACE_SYS_APS_BUDGETS(id,percent,critical)
#define _TRACE_SYS_APS_BANKRUPTCY(id,pid,tid)
#define _TRACE_SYS_EMIT_ADDRESS(thp,vaddr)

#endif

#define __KERTRACE_H_INCLUDED
#endif

/* __SRCVERSION("kertrace.h $Rev: 207484 $"); */
