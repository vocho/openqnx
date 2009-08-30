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
 * Read one byte using the appropriate access privilege
 */
void
rd_probe_1(const void *p)
{
	const volatile int	*ptr = p;
	int					tmp;

	if (actives[KERNCPU]->process->boundry_addr == VM_KERN_SPACE_BOUNDRY) {
		__asm__ __volatile__ (
			"ldr	%0, [%1]"
			: "=r" (tmp)
			: "r"  (ptr)
		);
	} else {
		__asm__ __volatile__ (
			"ldrt	%0, [%1]"
			: "=&r" (tmp)
			: "r"  (ptr)
		);
	}
}

/*
 * Write one byte using the appropriate access privilege
 */
void
wr_probe_1(void *p)
{
	volatile int	*ptr = p;
	int				tmp;

	if (actives[KERNCPU]->process->boundry_addr == VM_KERN_SPACE_BOUNDRY) {
		__asm__ __volatile__ (
			"ldr	%0, [%1];"
			"str	%0, [%1];"
			: "=&r" (tmp)
			: "r" (ptr)
		);
	} else {
		__asm__ __volatile__ (
			"ldrt	%0, [%1];"
			"strt	%0, [%1];"
			: "=&r" (tmp)
			: "r" (ptr)
		);
	}
}

/*
 * Read num bytes using the appropriate access privilege
 */
void
rd_probe_num(const void *p, int num)
{
	const volatile int	*ptr = p;
	int					tmp;

	if (actives[KERNCPU]->process->boundry_addr == VM_KERN_SPACE_BOUNDRY) {
		__asm__ __volatile__ (
			"0:	ldr		%0, [%1], #4;"
			"	subs	%2, %2, #1;"
			"	bne		0b;"
			: "=&r" (tmp)
			: "r"  (ptr), "r" (num)
		);
	} else {
		__asm__ __volatile__ (
			"0:	ldrt	%0, [%1], #4;"
			"	subs	%2, %2, #1;"
			"	bne		0b;"
			: "=&r" (tmp)
			: "r" (ptr), "r" (num)
		);
	}
}

/*
 * Write num bytes using the appropriate access privilege
 */
void
wr_probe_num(void *p, int num)
{
	volatile int	*ptr = p;
	int				tmp;

	if (actives[KERNCPU]->process->boundry_addr == VM_KERN_SPACE_BOUNDRY) {
		__asm__ __volatile__ (
			"0:	ldr		%0, [%1];"
			"	str		%0, [%1], #4;"
			"	subs	%2, %2, #1;"
			"	bne		0b;"
			: "=&r" (tmp)
			: "r"  (ptr), "r" (num)
		);
	} else {
		__asm__ __volatile__ (
			"0:	ldrt	%0, [%1];"
			"	strt	%0, [%1], #4;"
			"	subs	%2, %2, #1;"
			"	bne		0b;"
			: "=&r" (tmp)
			: "r"  (ptr), "r" (num)
		);
	}
}

void
halt(void)
{
	/*
	 * FIXME: we use the currently unused power callout to invoke a board or
	 *        CPU-specific halt function.
	 *        If this callout is used for CPU-independent power management,
	 *        we will have to use the appropriate argument, and then change
	 *        all the startup power callouts.
	 */
	if (calloutptr->power) {
		(void) calloutptr->power(_syspage_ptr, 0, 0);
	}
}

/*
 * CPU specific initialization of syspage pointers & stuff
 */
void
cpu_syspage_init(void)
{
	/*
	 * FIXME: check whether we should do anything here...
	 */
}

/*
 * Copy register context, but make sure there are no security holes
 */
void
cpu_greg_load(THREAD *thp, CPU_REGISTERS *regs)
{
	unsigned	mode = thp->reg.spsr & 0x0fffffff;

	thp->reg = *regs;
	thp->reg.spsr = (regs->spsr & 0xf0000000) | mode;
}

/*
 * Initialize a thread's registers
 */
void
cpu_thread_init(THREAD *act, THREAD *thp, int align)
{
	/*
	 * Set the appropriate processor mode for thread execution
	 */
	if (act == NULL || act->process == sysmgr_prp) {
		/*
		 * Create privileged thread
		 */
		thp->reg.spsr = arm_cpuid_arch() ? ARM_CPSR_MODE_SYS : ARM_CPSR_MODE_SVC;
	}
	else {
		/*
		 * Inherit the privilege of our creator
		 */
		thp->reg.spsr = act->reg.spsr & ARM_CPSR_MODE_MASK;
	}

	/*
	 * Set appropriate behaviour for alignment fault handling
	 */
	switch(align) {
	case 0:	// default case causes trap
	case 1:
		thp->flags |= _NTO_TF_ALIGN_FAULT;
		break;
	default: break;
	}
}


/*
 * Set up thread to call tls->__exitfunc(tls->__arg) if entry function returns.
 */
void
cpu_thread_waaa(THREAD *thp)
{
	thp->reg.gpr[ARM_REG_LR] = (uintptr_t)thp->un.lcl.tls->__exitfunc;
	thp->reg.gpr[ARM_REG_R0] = (uintptr_t)thp->un.lcl.tls->__arg;
}


/*
 * Allow thread to perform privileged operations.
 */
void
cpu_thread_priv(THREAD *thp)
{
	unsigned	spsr = thp->reg.spsr & ~ARM_CPSR_MODE_MASK;

	thp->reg.spsr = spsr | (arm_cpuid_arch() ? ARM_CPSR_MODE_SYS : ARM_CPSR_MODE_SVC);
}


/*
 * Allow/disallow alignment faults on a thread
 *
 * The CPU always generates alignment faults, and the data abort handler
 * checks _NTO_TF_ALIGN_FAULT to determine whether to trap or emulate.
 */
void
cpu_thread_align_fault(THREAD *thp)
{
	// Nothing to be done on ARM
}

/*
 * CPU specific stuff for destroying a thread
 */
void
cpu_thread_destroy(THREAD *thp)
{
	// Nothing to be done on ARM
}


/*
 * Save registers trashed during signal delivery.
 *
 *	r0   - is used to pass _sighandler_info * to __signalstub
 *	lr   - is used as a temporary by __signalstub
 *	spsr - can't be accessed by __signalstub
 *
 *	signal_specret() saves sp and pc.
 *
 * __signalstub saves r1-r12 (see libc/1/arm/sigstub.S)
 */
void
cpu_signal_save(SIGSTACK *ssp, THREAD *thp)
{
	ucontext_t	*uc;

	uc = ssp->info.context;
	uc->uc_mcontext.cpu.gpr[ARM_REG_R0] = thp->reg.gpr[ARM_REG_R0];
	uc->uc_mcontext.cpu.gpr[ARM_REG_LR] = thp->reg.gpr[ARM_REG_LR];
	uc->uc_mcontext.cpu.spsr			= thp->reg.spsr;

	/*
	 * __signalstub is ARM code - make sure we return to ARM mode
	 */
	thp->reg.spsr &= ~(ARM_CPSR_T|ARM_CPSR_J);
}


/*
 * Restore registers on return from signal handler.
 *
 *	r0   - was trashed in order to make the SignalReturn system call.
 *  ip   - was trashed in order to make the SignalReturn system call.
 *  lr   - will have been trashed during the signal handling.
 *  spsr - will have been trashed if interrupts/system calls occurred.
 *
 * ker_signal_return() restores sp and pc.
 */
void
cpu_signal_restore(THREAD *thp, SIGSTACK *ssp)
{
	ucontext_t	*uc;
	unsigned	spsr;

	uc = ssp->info.context;
	thp->reg.gpr[ARM_REG_R0] = uc->uc_mcontext.cpu.gpr[ARM_REG_R0];
	thp->reg.gpr[ARM_REG_IP] = uc->uc_mcontext.cpu.gpr[ARM_REG_IP];
	thp->reg.gpr[ARM_REG_LR] = uc->uc_mcontext.cpu.gpr[ARM_REG_LR];
	spsr = uc->uc_mcontext.cpu.spsr & ~ARM_CPSR_MODE_MASK;
	thp->reg.spsr = spsr | (thp->reg.spsr & ARM_CPSR_MODE_MASK);
}


/*
 * CPU specific stuff for saving registers needed for interrupt delivery
 */
void
cpu_intr_attach(INTERRUPT *itp, THREAD *thp)
{
	// Nothing to be done for ARM
}


/*
 * CPU specific stuff for initializing a process's registers
 */
void
cpu_process_startup(THREAD *thp, int forking)
{
	if(!forking) {
		thp->reg.gpr[ARM_REG_R0] = 0;		// at_exit pointer

		/*
		 * Force user mode execution
		 */
		thp->reg.spsr = ARM_CPSR_MODE_USR;
	}
}


/*
 * CPU specific stuff for cleaning up before rebooting
 */
void
cpu_reboot(void)
{
	/*
	 * FIXME: need to think about what needs doing
	 */
}


/*
 * Get next AP in an SMP system initialized and into the kernel.
 */
void
cpu_start_ap(uintptr_t start)
{
	if(_syspage_ptr->smp.entry_size > 0) {
		SYSPAGE_ENTRY(smp)->start_addr = (void (*)())start;
	}
}


/*
 * Force a thread to execute the destroyall kernel call.
 */
void
cpu_force_thread_destroyall(THREAD *thp)
{
	/*
	 * kercallptr is ARM code - make sure we return to ARM mode
	 */
	thp->reg.spsr &= ~(ARM_CPSR_T|ARM_CPSR_J);

	thp->reg.gpr[ARM_REG_IP] = __KER_THREAD_DESTROYALL;
	SETKIP(thp, kercallptr);
}

void
cpu_force_fpu_save(THREAD *thp)
{
	FPU_REGISTERS	*fpu = FPUDATA_PTR(thp->fpudata);

	fpu_ctx_save(fpu);
#ifdef	VARIANT_smp
	thp->fpudata = fpu;	// clear BUSY/CPU bits
	fpu_disable();
#endif
}

/*
 * Rewind PC for mutex critical region
 */
void
cpu_mutex_adjust(THREAD *act)
{
	if (KSP(act) & 1) {
		uintptr_t	start = act->reg.gpr[ARM_REG_IP];
		uintptr_t	end   = act->reg.gpr[ARM_REG_LR];
		uintptr_t	pc = KIP(act);

		if (pc >= start && pc < end && end - start < 20) {
			SETKIP(act, start);
		}
	}
}

__SRCVERSION("cpu_misc.c $Rev: 211269 $");
