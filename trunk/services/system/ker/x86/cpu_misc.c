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

#include "externs.h"

/*
 * CPU specific initialization of syspage pointers & stuff
 */
void
cpu_syspage_init(void) {
	realmode_addr = (uintptr_t)_syspage_ptr->un.x86.real_addr;
}

/*
 * CPU specific stuff for initializing a thread's registers
 */
void
cpu_thread_init(THREAD *act, THREAD *thp, int align) {
	thp->reg.efl = X86_PSW_IF | (1 << X86_PSW_IOPL_SHIFT);
	if(act != NULL) {
		thp->reg.efl |= act->reg.efl & X86_PSW_IOPL_MASK;
	}
	if(act == NULL || act->process == sysmgr_prp) {
		thp->reg.cs = sys_cs;
		thp->reg.ss = sys_ss;
#ifdef __SEGMENTS__
		thp->reg.ds = sys_ds;
		thp->reg.es = sys_ds;
#endif
	} else {
		thp->reg.cs = usr_cs;
		thp->reg.ss = usr_ss;
#ifdef __SEGMENTS__
		thp->reg.ds = usr_ds;
		thp->reg.es = usr_ds;
#endif
	}
	if(align == 1) {
		thp->flags |= _NTO_TF_ALIGN_FAULT;
		thp->reg.efl |= X86_PSW_AC;
	}
	thp->cpu.pcr = &disabled_perfregs; /* invalid part id */
}

/*
 * CPU specific stuff for putting a thread into execution for the
 * first time. We have to arrange a stack frame such that the function
 * thinks it needs to return to thp->args.wa->exitfunc, and that it has
 * been passed thp->args.wa.arg.
 *
 * As it happens, this is already done on the x86, because of the
 * position of the thread local storage at the top of the thread's stack.
 */
void
cpu_thread_waaa(THREAD *thp) {
	// We set the task switched bit before a new thread runs.
	// This will ensure a trap if it attempts to use the fpu
	// even if nobody else is using it.
	setts();
}

/*
 * CPU specific stuff for allowing a thread to do priviledged operations
 */
void
cpu_thread_priv(THREAD *thp) {
	thp->reg.efl |= 0x3 << X86_PSW_IOPL_SHIFT;
}

/*
 * CPU specific stuff for destroying a thread
 */
void
cpu_thread_destroy(THREAD *thp) {
	cpu_free_perfregs(thp);
	/* Nothing needed */
}

/*
 * CPU specific stuff for allowing/disallowing alignment faults on a thread
 */
void
cpu_thread_align_fault(THREAD *thp) {
    if(thp->flags & _NTO_TF_ALIGN_FAULT) {
		thp->reg.efl |= X86_PSW_AC;
	} else {
		thp->reg.efl &= ~X86_PSW_AC;
	}
}

/*
 * CPU specific stuff for saving registers on signal delivery
 */
void
cpu_signal_save(SIGSTACK *ssp, THREAD *thp) {
	ucontext_t	*uc;

	uc = ssp->info.context;
	uc->uc_mcontext.cpu.eax = thp->reg.eax;
	uc->uc_mcontext.cpu.efl = thp->reg.efl & ~SYSENTER_EFLAGS_BIT;
	uc->uc_mcontext.cpu.cs  = thp->reg.cs;
	uc->uc_mcontext.cpu.ss  = thp->reg.ss;

	lock_kernel();
	thp->reg.efl = X86_PSW_IF | (thp->reg.efl & X86_PSW_IOPL_MASK);
}

/*
 * CPU specific stuff for restoring from a signal
 */
void
cpu_signal_restore(THREAD *thp, SIGSTACK *ssp) {
	ucontext_t	*uc;
	uint32_t	efl = thp->reg.efl;

	uc = ssp->info.context;
	thp->reg.eax = uc->uc_mcontext.cpu.eax;
    /*
	 * Don't restore IOPL level - user might have done a ThreadCtl in
	 * signal handler (it was also a security hole).
	 * Also don't let them turn on the SYSENTER_EFLAGS_BIT
	 */
	thp->reg.efl = (uc->uc_mcontext.cpu.efl & ~(X86_PSW_IOPL_MASK|X86_PSW_TF|SYSENTER_EFLAGS_BIT))
					| (efl & (X86_PSW_IOPL_MASK|X86_PSW_TF));
}

/*
 * CPU specific stuff for saving registers needed for interrupt delivery
 */
void
cpu_intr_attach(INTERRUPT *itp, THREAD *thp) {
	/* Nothing needed for X86 */
}

/*
 * This copies the register sets, but makes sure there are no
 * security holes
 */
void cpu_greg_load(THREAD *thp, CPU_REGISTERS *regs) {
	uint32_t		efl = thp->reg.efl;
	uint32_t		cs = thp->reg.cs;
	uint32_t		ss = thp->reg.ss;
#ifdef __SEGMENTS__
	uint32_t		ds = thp->reg.ds;
	uint32_t		es = thp->reg.es;
	uint32_t		fs = thp->reg.fs;
	uint32_t		gs = thp->reg.gs;
#endif

	thp->reg = *regs;
    /*
	 * Don't restore IOPL level or segments, it is a security hole).
	 */
	thp->reg.efl = (regs->efl & ~X86_PSW_IOPL_MASK) | (efl & X86_PSW_IOPL_MASK);
	thp->reg.cs = cs;
	thp->reg.ss = ss;
#ifdef __SEGMENTS__
	thp->reg.ds = ds;
	thp->reg.es = es;
	thp->reg.fs = fs;
	thp->reg.gs = gs;
#endif
}

void
cpu_process_startup(THREAD *thp, int forking) {
	if(!forking) {
		thp->reg.ebx = thp->reg.edx = 0;
		thp->reg.ebp = 0;
		thp->reg.cs = usr_cs;
		thp->reg.ss = usr_ss;
#ifdef __SEGMENTS__
		thp->reg.ds = thp->reg.es = usr_ds;
		thp->reg.fs = thp->reg.gs = 0;
#endif
	}
}

/*
 * cpu_reboot(void)
 *	CPU specific stuff for cleaning up before rebooting
 */
void
cpu_reboot(void) {
}

/*
 * cpu_start_ap(void)
 *	CPU specific stuff for getting next AP in an SMP system initialized
 *	and into the kernel.
 */
void
cpu_start_ap(uintptr_t start) {
	if(_syspage_ptr->smp.entry_size > 0) {
		SYSPAGE_ENTRY(smp)->start_addr = (void *)start;
	} else if(_syspage_ptr->un.x86.smpinfo.entry_size > 0) {
		//
		// This code can be removed later, after all the startup's have
		// been updated to use the new CPU independent smp section.
		//
		SYSPAGE_CPU_ENTRY(x86, smpinfo)->ap_start_addr = (void *)start;
	}
}

/*
 * CPU specific code to force a thread to execute the destroyall kernel
 * call.
 */
void
cpu_force_thread_destroyall(THREAD *thp) {
	thp->reg.eax = __KER_THREAD_DESTROYALL;
	SETKIP(thp, kercallptr);
}

__SRCVERSION("cpu_misc.c $Rev: 199160 $");
