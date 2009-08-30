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
#include <hw/inout.h>

/*
 * CPU specific initialization of syspage pointers & stuff
 */
void
cpu_syspage_init(void) {
	unsigned		i;

	/* get the flags more accessable for use in kernel.s */
	if(__cpu_flags & CPU_FLAG_MMU) {
		privateptr->user_cpupageptr = (void *)VM_CPUPAGE_ADDR;
		privateptr->user_syspageptr = (void *)VM_SYSPAGE_ADDR;
		for(i = 0; i < NUM_PROCESSORS; ++i) {
			cpupageptr[i]->syspage = (void *)VM_SYSPAGE_ADDR;
		}
		kercallptr = (void *)(VM_SYSPAGE_ADDR
				+ (uintptr_t)kercallptr - (uintptr_t)_syspage_ptr);
	}
}

/*
 * CPU specific stuff for initializing a thread's registers
 */
void
cpu_thread_init(THREAD *act, THREAD *thp, int align) {
	thp->reg.msr = SYSPAGE_CPU_ENTRY(ppc,kerinfo)->init_msr | PPC_MSR_PR;
	if(__cpu_flags & CPU_FLAG_MMU) {
		thp->reg.msr |= PPC_MSR_IR | PPC_MSR_DR;
	}
	if(__cpu_flags & CPU_FLAG_FPU) {
		thp->reg.msr |= PPC_MSR_FE0 | PPC_MSR_FE1;
	}
	if(act != NULL) {
		thp->reg.msr &= ((act->reg.msr & PPC_MSR_PR) | (~PPC_MSR_PR));
		thp->reg.gpr[1] -= STACK_ALIGNMENT;
		thp->reg.gpr[2] = act->reg.gpr[2];
		thp->reg.gpr[13] = act->reg.gpr[13];
		thp->cpu.low_mem_boundry = act->cpu.low_mem_boundry;
	} else {
		extern uint8_t	_SDA_BASE_[];
		extern uint8_t	_SDA2_BASE_[];

		thp->reg.msr &= ~PPC_MSR_PR;
		thp->reg.gpr[2] = (uintptr_t) _SDA2_BASE_;
		thp->reg.gpr[13] = (uintptr_t) _SDA_BASE_;
		thp->cpu.low_mem_boundry = VM_KERN_LOW_SIZE;
	}
#if defined(VARIANT_900)
	if((act == NULL) || (act->process == sysmgr_prp)) {
		thp->reg.u.msr_u = PPC64_MSR_HV >> 32;
	}
#endif	
	switch(align) {
	case 1:
		thp->flags |= _NTO_TF_ALIGN_FAULT;
		break;
	default:
		break;
	}
	thp->cpu.pcr = &disabled_perfregs; /* invalid part id */
}

/*
 * CPU specific stuff for putting a thread into execution for the
 * first time. We have to arrange a stack frame such that the function
 * thinks it needs to return to thp->args.wa->exitfunc and that it has
 * been passed thp->args.wa.arg.
 */
void
cpu_thread_waaa(THREAD *thp) {
	thp->reg.lr = (uintptr_t)thp->un.lcl.tls->__exitfunc;
	if(thp->args.wa.not_sigev_thread) {
		thp->reg.gpr[3] = (uintptr_t)thp->un.lcl.tls->__arg;
	} else {
		/*
			A thread created by SIGEV_THREAD. Rather than a pointer,
			it's passed a "union sigval". Same size as a pointer, but the
			PPC ABI passes all structs/unions as pointers to a temporary,
			no matter what size they are.
		*/
		thp->reg.gpr[3] = (uintptr_t)&thp->un.lcl.tls->__arg;
	}
}

/*
 * CPU specific stuff for allowing a thread to do priviledged operations
 */
void
cpu_thread_priv(THREAD *thp) {
	thp->reg.msr &= ~PPC_MSR_PR;
}

/*
 * CPU specific stuff for allowing/disallowing alignment faults on a thread
 */
void
cpu_thread_align_fault(THREAD *thp) {
	/* Nothing needed for PPC */
}

/*
 * CPU specific stuff for destroying a thread
 */

void
cpu_thread_destroy(THREAD *thp) {
	/* Check for Altivec register save area */
	if(thp->cpu.alt) {
		alt_context_free(thp);
	}
	cpu_free_perfregs(thp);
}

/*
 * CPU specific stuff for saving registers on signal delivery
 */
void
cpu_signal_save(SIGSTACK *ssp, THREAD *thp) {
	ucontext_t	*uc;

	uc = ssp->info.context;
	uc->uc_mcontext.cpu.gpr[0] = thp->reg.gpr[0];
	uc->uc_mcontext.cpu.gpr[3] = thp->reg.gpr[3];
	uc->uc_mcontext.cpu.gpr[11]= thp->reg.gpr[11];
	uc->uc_mcontext.cpu.gpr[12]= thp->reg.gpr[12];
	uc->uc_mcontext.cpu.ctr    = thp->reg.ctr;
	uc->uc_mcontext.cpu.cr     = thp->reg.cr;
	uc->uc_mcontext.cpu.xer    = thp->reg.xer;
	uc->uc_mcontext.cpu.msr    = thp->reg.msr;

	lock_kernel();
	// This next statement is to indicate to __signalstub() that
	// We've saved R11, R12 and CTR. It's an 'impossible' condition code
	// that no instruction can ever set. 
	thp->reg.cr |= 0xf;
}

/*
 * CPU specific stuff for restoring from a signal
 */
void
cpu_signal_restore(THREAD *thp, SIGSTACK *ssp) {
	ucontext_t	*uc;

	uc = ssp->info.context;
	thp->reg.gpr[0] = uc->uc_mcontext.cpu.gpr[0];
	thp->reg.gpr[3] = uc->uc_mcontext.cpu.gpr[3];
	thp->reg.gpr[11]= uc->uc_mcontext.cpu.gpr[11];
	thp->reg.gpr[12]= uc->uc_mcontext.cpu.gpr[12];
	thp->reg.ctr    = uc->uc_mcontext.cpu.ctr;
	thp->reg.cr     = uc->uc_mcontext.cpu.cr;
	thp->reg.xer    = uc->uc_mcontext.cpu.xer;
	thp->reg.msr    = (thp->reg.msr & ~ppc_ienable_bits)
					    | (uc->uc_mcontext.cpu.msr & ppc_ienable_bits);
}

/*
 * CPU specific stuff for saving registers needed for interrupt delivery
 */
void
cpu_intr_attach(INTERRUPT *itp, THREAD *thp) {
	itp->cpu.gpr2 = thp->reg.gpr[2];
	itp->cpu.gpr13 = thp->reg.gpr[13];
}

/*
 * This copies the register sets, but makes sure there are no
 * security holes
 */
void cpu_greg_load(THREAD *thp, CPU_REGISTERS *regs) {
	uint32_t		msr = thp->reg.msr;

	thp->reg = *regs;
    /*
	 * Don't restore MSR_PR, it is a security hole
	 */
	thp->reg.msr = (regs->msr & ~PPC_MSR_PR) | (msr & PPC_MSR_PR);
}

/*
 * CPU specific stuff for initializing a process's registers
 */
void
cpu_process_startup(THREAD *thp, int forking) {
	void		**p;

	thp->cpu.low_mem_boundry = 0;
	// Reset kcall flag here so that all registers are restored properly
	thp->flags &= ~_NTO_TF_KCALL_ACTIVE;
	if(!forking) {
		thp->reg.gpr[3] = *(unsigned *)thp->reg.gpr[1]; 		/* argc */
		thp->reg.gpr[4] = thp->reg.gpr[1] + sizeof( unsigned ); /* argv */
		thp->reg.gpr[5] = thp->reg.gpr[4] + (thp->reg.gpr[3]+1) * sizeof( uintptr_t ); /* envp */
		p = (void **)thp->reg.gpr[5];
		while(*p != NULL) ++p;
		thp->reg.gpr[6] = (uintptr_t)(p + 1); /* auxiliary vector */
		thp->reg.gpr[1] -= STACK_ALIGNMENT;
		thp->reg.msr |= PPC_MSR_PR;
	}
}

/*
 * CPU specific stuff for cleaning up before rebooting
 */
void
cpu_reboot(void) {
}


/*
 * cpu_start_ap(void *)
 *	CPU specific stuff for getting next AP in an SMP system initialized
 *	and into the kernel.
 */
void
cpu_start_ap(uintptr_t start_func) {
	void	**start = NULL;

	if(_syspage_ptr->smp.entry_size > 0) {
		start = &SYSPAGE_ENTRY(smp)->start_addr;
	} else if(_syspage_ptr->un.ppc.smpinfo.entry_size > 0) {
		//
		// This code can be removed later, after all the startup's have
		// been updated to use the new CPU independent smp section.
		//
		start = &SYSPAGE_CPU_ENTRY(ppc,smpinfo)->mpu_start_addr;
	}
	if(start != NULL) {
		*start = (void *)start_func;
		// The data cache flush is done because the AP's spining around
		// on the syspage don't currently have their data cache enabled.
		// Once things are fixed up so that caches gets turned on at the
		// begining of startup like they should, this next line can go
		// away.
		__asm__ __volatile__( "dcbst 0,%0" :: "b" (start) );
		mem_barrier(); // make sure the store gets done.
	}
}

/*
 * CPU specific code to force a thread to execute the destroyall kernel
 * call.
 */
void
cpu_force_thread_destroyall(THREAD *thp) {
	thp->reg.gpr[0] = __KER_THREAD_DESTROYALL;
	SETKIP(thp, kercallptr);
}

__SRCVERSION("cpu_misc.c $Rev: 153052 $");
