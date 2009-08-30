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
 * cpu_misc.c
 *	Miscellaneous CPU-ish services
 *
 * Things will show up here if they're x86 CPU services, too.
 */
#include "externs.h"
#include <mips/vm.h>
#include <mips/sb1cpu.h>

/*
 * halt()
 *	No such thing, so just return to enable spinning
 */
void
halt(void) {
}

void sb1_cache_clean(paddr_t bad_paddr);

/*
 * cache_error()
 *	Parse the COP0 cache_error register and report 
 *	the details of the cache exception. Then either crash
 * 	or try to recover and continue from the error.
 */
void
cache_error(void) {
#if defined(VARIANT_r3k)
	crash();
#else
    uint32_t cerr;
    uint32_t err_epc;
	int    imprecise_exception_errata = 0;
	uint32_t sreg = getcp0_sreg();
	uint32_t epc = CP0REG_GET(14); /* getcp0_epc(); */

    /* disable further cache errors */
    setcp0_sreg(sreg | MIPS_SREG_DE);

    err_epc = getcp0_err_epc();
    cerr = getcp0_cerr();

	/* if the ErrEPC is the General Exception Vector or the
	 * EPC is the Cache Exception Vector, then we have been trapped by the
	 * infamous "Simultaneous Imprecise Exception Errata", which so far
	 * affects both the Sandcraft SR7100 and the Broadcom SB-1
	 */
	if ( err_epc == MIPS_EXCV_GENERAL || epc == (MIPS_R4K_K1BASE|MIPS_EXCV_CACHEERR) ) {
		imprecise_exception_errata = 1;
	}
	if(MIPS_PRID_COMPANY(getcp0_prid()) == MIPS_PRID_COMPANY_BROADCOM) {
		/* Broadcom SB-1 has specialised cache error registers */
		uint32_t cerr_d = CP0REG_SEL_GET(27,1);
		uint32_t errctl = CP0REG_SEL_GET(26,0);
		uint32_t cerr_dpa = CP0REG_SEL_GET(27,3); /* getcp0_cerr_dpa() */
		if ( errctl & SB_CERR_INST ) {
			kprintf("\n*** Cache Error Exception at 0x%x, Instruction Cache ErrCtl 0x%x CacheErr-I 0x%x ***\n", 
								err_epc, errctl, cerr);
			kprintf("%s error, %s cache",
				   (cerr & SB_I_TAG) ? "tag_parity" : "data_array",
				   (cerr & SB_I_EXTERNAL) ? "external" : "primary" );
		} else if ( errctl & SB_CERR_DATA ) {
			/* always _try_ to clean out cache data */
			sb1_cache_clean(cerr_dpa);
			kprintf("\n*** Cache Error Exception at 0x%x, Data Cache ErrCtl 0x%x CacheErr-D 0x%x ***\n", 
								err_epc, errctl, cerr_d);
			if ( cerr_d & SB_D_MUTIPLE ) {
				kprintf("multiple ");
			}
			if ( cerr_d & SB_D_TAG_STATE_PARITY ) {
				kprintf("tag_state_parity ");
			}
			if ( cerr_d & SB_D_TAG_ADDR_PARITY ) {
				kprintf("tag_address_parity ");
			}
			if ( cerr_d & SB_D_TAG_DATA_SINGLE_ERR ) {
				kprintf("tag_data_single ");
			}
			if ( cerr_d & SB_D_TAG_DATA_DOUBLE_ERR ) {
				kprintf("tag_data_double ");
			}
			if ( cerr_d & SB_D_EXTERNAL_ERR ) {
				kprintf("external ");
			}
			if ( cerr_d & SB_D_LOAD_ERR ) {
				kprintf("load ");
			}
			if ( cerr_d & SB_D_STORE_ERR ) {
				kprintf("store ");
			}
			if ( cerr_d & SB_D_FILL_ERR ) {
				kprintf("fill ");
			}
			if ( cerr_d & SB_D_COH_ERR ) {
				kprintf("coherency ");
			}
			if ( cerr_d & SB_D_DT_ERR ) {
				kprintf("dt ");
			}
			kprintf("error, CacheErr-DPA=0x%x\n", cerr_dpa );
		} else {
			kprintf("\n*** Cache Error Exception at 0x%x, errctl 0x%x ***\n", 
								err_epc, errctl);
		}
  } else {
    kprintf("\n*** Cache Error Exception at 0x%x, cerr 0x%x epc 0x%x***\n", 
						err_epc, cerr, epc);
    kprintf("%s reference, %s cache, %s %s",
           (cerr & MIPS_CACHE_ERR_ER) ? "data" : "instruction",
           (cerr & MIPS_CACHE_ERR_EC) ? "reserved" : "primary",
           (cerr & MIPS_CACHE_ERR_ED) ? "data field error" : "",
           (cerr & MIPS_CACHE_ERR_ET) ? ", tag field error" : "" );
    if (cerr & MIPS_CACHE_ERR_ES) {
        kprintf(", cache miss parity error %s SysAD Bus %s\n",
               (cerr & MIPS_CACHE_ERR_EE) ? "on" : "not on",
               (cerr & MIPS_CACHE_ERR_EB) ? ", data error also occurred" : "");
    } else {
        kprintf(", error %s SysAD Bus %s\n",
               (cerr & MIPS_CACHE_ERR_EE) ? "on" : "not on",
               (cerr & MIPS_CACHE_ERR_EB) ? ", data error also occurred" : "");
    }
		}
  if ( imprecise_exception_errata ) {
    kprintf("*** Imprecise Exception Errata Detected: ErrEPC=0x%x EPC=0x%x ***\n", err_epc, epc );
  }

  crash();
#endif /* !defined(VARIANT_r3k) */
}


/*
 * cpu_syspage_init
 *	Set up kernel parts of syspage
 */
void
cpu_syspage_init(void) {
	__shadow_imask = &_syspage_ptr->un.mips.shadow_imask;
	sys_kercallptr = &SYSPAGE_ENTRY(system_private)->kercall;

#if defined(VARIANT_32)
	SYSPAGE_ENTRY(cpuinfo)->flags &= ~(MIPS_CPU_FLAG_64BIT|MIPS_CPU_FLAG_128BIT);
	__cpu_flags = SYSPAGE_ENTRY(cpuinfo)->flags;
#elif !defined(VARIANT_tx79)
	SYSPAGE_ENTRY(cpuinfo)->flags &= ~(MIPS_CPU_FLAG_128BIT);
	__cpu_flags = SYSPAGE_ENTRY(cpuinfo)->flags;
#endif
}

/*
 * cpu_thread_align_fault()
 *	Does nothing on MIPS
 */
void
cpu_thread_align_fault(THREAD *thp) {
}

/*
 * cpu_thread_waaa()
 *	Set up thread so it will be ready to return to "exitfunc"
 *
 * Execution will start at a system independent determined place,
 * and will think it has been passed the argument "wa.arg".
 */
void
cpu_thread_waaa(THREAD *thp) {
	MIPS_REG_SETx64(&thp->reg, MIPS_REG_RA, thp->un.lcl.tls->__exitfunc);
	MIPS_REG_SETx64(&thp->reg, MIPS_REG_A0, thp->un.lcl.tls->__arg);
}

/*
 * cpu_thread_priv()
 *	Enable a thread to access privileged resources
 */
void
cpu_thread_priv(THREAD *thp) {
	thp->reg.regs[MIPS_CREG(MIPS_REG_SREG)] |= MIPS_SREG_CU0;
}

/*
 * CPU specific stuff for destroying a thread
 */
void
cpu_thread_destroy(THREAD *thp) {
	/* Nothing needed */
	cpu_free_perfregs(thp);
}

#define SAVE_REG( thp, idx, save ) \
		((save) = *(uint64_t *)&(thp)->reg.regs[MIPS_BAREG(idx)])
#define RESTORE_REG( thp, idx, restore ) \
		(*(uint64_t *)&(thp)->reg.regs[MIPS_BAREG(idx)] = (restore))
/*
 * cpu_intr_attach()
 *	CPU specific stuff for saving registers needed for interrupt delivery
 */
void
cpu_intr_attach(INTERRUPT *itp, THREAD *thp) {
	SAVE_REG(thp, MIPS_REG_GP, itp->cpu.gp);
}

#define MOVE_REG(dst, src, idx)	\
	(dst)->regs[MIPS_BAREG(idx)+0] = (src)->regs[MIPS_BAREG(idx)+0];	\
	(dst)->regs[MIPS_BAREG(idx)+1] = (src)->regs[MIPS_BAREG(idx)+1];

/*
 * cpu_signal_save()
 *	Save registers damaged by signal delivery
 *
 * Note that this style requires that the signal handler be fronted by
 * assembly code to save the registers which a callee is free to use.
 * But it's better to have this implemented and executed in the user
 * context than here in the kernel.
 */
void
cpu_signal_save(SIGSTACK *ssp, THREAD *thp) {
	ucontext_t	*uc;

	uc = ssp->info.context;
	MOVE_REG(&uc->uc_mcontext.cpu, &thp->reg, MIPS_REG_A0);
	MOVE_REG(&uc->uc_mcontext.cpu, &thp->reg, MIPS_REG_V0);
	MOVE_REG(&uc->uc_mcontext.cpu, &thp->reg, MIPS_REG_AT);
}

/*
 * cpu_signal_restore()
 *	Restore any registers perturbed by signal delivery
 */
void
cpu_signal_restore(THREAD *thp, SIGSTACK *ssp) {
	ucontext_t	*uc;

	uc = ssp->info.context;

	MOVE_REG(&thp->reg, &uc->uc_mcontext.cpu, MIPS_REG_A0);
	MOVE_REG(&thp->reg, &uc->uc_mcontext.cpu, MIPS_REG_V0);
	MOVE_REG(&thp->reg, &uc->uc_mcontext.cpu, MIPS_REG_AT);
}

/*
 * cpu_thread_init()
 *	Set up a new thread's registers
 *
 * Alignment is ignored.
 */
void
cpu_thread_init(THREAD *act, THREAD *thp, int align) {
	MIPS_CPU_REGISTERS *r = &thp->reg;
	unsigned long sreg;
	extern unsigned char _gp[];

	/*
	 * Set for kernel execution if we're running unmapped
	 * or we're the process manager, otherwise run in supervisor/user
	 * mode.
	 */
	if(act) {
		sreg = act->reg.regs[MIPS_CREG(MIPS_REG_SREG)];
	} else {
		sreg = getcp0_sreg();
	}
 	if(!act || (act->process == sysmgr_prp) || !(__cpu_flags & CPU_FLAG_MMU)) {
#if defined(VARIANT_r3k)
		sreg &= ~(MIPS3K_SREG_KUo|MIPS3K_SREG_KUp|MIPS3K_SREG_KUc);
#else
		sreg &= ~MIPS_SREG_KSU;
	} else if((sreg & MIPS_SREG_KSU) != MIPS_SREG_MODE_SUPER) {
		//
		//If the active thread is in supervisor mode, its process
		//asked for access to KSEG2 at some point in the past. Keep
		//the new thread in supervisor mode as well so it can get
		//to KSEG2 as well.
		//
		sreg |= (MIPS_SREG_MODE_USER | MIPS_SREG_UX | MIPS_SREG_SX);
#endif
	}

	/*
	 * Enable external interrupts.
	 */
#if defined(VARIANT_r3k)
	sreg |= MIPS3K_SREG_IEp;
#else
	sreg |= MIPS_SREG_IE;
#endif

	/*
	 * If there's an active thread, set the new GP to reflect his and
	 * set T9 to point at initial thread function, in case we're calling
	 * PIC-compiled code.
	 */
	if (act) {
		MIPS_REG_SETx64(r, MIPS_REG_T9, REGIP(r));	
		MIPS_REG_SETx64(r, MIPS_REG_GP, act->reg.regs[MIPS_CREG(MIPS_REG_GP)]);
		sreg &= ~MIPS_SREG_CU1;
	} else {
		MIPS_REG_SETx64(r, MIPS_REG_GP, _gp);
		sreg |= MIPS_SREG_CU1;
	}

	r->regs[MIPS_CREG(MIPS_REG_SREG)] = sreg;

	switch(align) {
	case 0:
	case 1:
		thp->flags |= _NTO_TF_ALIGN_FAULT;
		break;
	default: break;
	}
	thp->cpu.pcr = &disabled_perfregs; /* invalid part id */
}

/*
 * This copies the register sets, but makes sure there are no
 * security holes
 */
void cpu_greg_load(THREAD *thp, CPU_REGISTERS *regs) {
	unsigned sreg = thp->reg.regs[MIPS_CREG(MIPS_REG_SREG)];

	thp->reg = *regs;
	thp->reg.regs[MIPS_CREG(MIPS_REG_SREG)] = sreg;
}

/*
 * cpu_process_startup()
 *	Set up a the thread for a newly created process
 */
void
cpu_process_startup(THREAD *thp, int forking) {
	MIPS_CPU_REGISTERS *r = &thp->reg;
 	unsigned sreg;
 
	if(!forking) {
		sreg = r->regs[MIPS_CREG(MIPS_REG_SREG)];

#if !defined(VARIANT_r3k)
		//NYI: Until we can figure out how to tell if the bit should be
		//on or not.
		sreg &= ~MIPS_SREG_FR;
#endif

		/*
		 * Make sure that the thread is running
		 * in user mode if it is not a kseg0 process.
		 */
		if(!MIPS_IS_KSEG0(REGIP(r))) {
			sreg &= ~(MIPS_SREG_CU1 | MIPS_SREG_CU0);
#if defined(VARIANT_r3k)
			sreg |= MIPS3K_SREG_KUo | MIPS3K_SREG_KUp;
#else
			sreg |= (MIPS_SREG_MODE_USER | MIPS_SREG_UX | MIPS_SREG_SX);
#endif
		}
		r->regs[MIPS_CREG(MIPS_REG_SREG)] = sreg;

		/*
		 * Set atexit() pointer to inactive
		 */
		MIPS_REG_SETx64(r, MIPS_REG_V0, 0);

		/*
		 * Set return address to NULL (ABI requirement, helps
		 * with stack backtrace)
		 */
		MIPS_REG_SETx64(r, MIPS_REG_RA, 0);
	}
}

/*
 * cpu_reboot(void)
 *	CPU specific stuff for cleaning up before rebooting
 */
void cpu_reboot(void) {

	InterruptDisable();
#if !defined(VARIANT_r3k)
	setcp0_sreg(getcp0_sreg() | MIPS_SREG_EXL);
	setcp0_sreg(getcp0_sreg() & ~(MIPS_SREG_ERL | MIPS_SREG_IMASK));
#endif
	/*
	 * Flush our caches, so the monitor doesn't have to deal
	 * with flotsam appearing in memory
	 */
	CacheControl(0UL, ~0UL, MS_SYNC);
}

/*
 * cpu_start_ap(void)
 *	CPU specific stuff for getting next AP in an SMP system initialized
 *	and into the kernel.
 */
void
cpu_start_ap(uintptr_t start) {
	if(_syspage_ptr->smp.entry_size > 0) {
		SYSPAGE_ENTRY(smp)->start_addr = (void (*)())start;
	}
}

/*
 * CPU specific code to force a thread to execute the destroyall kernel
 * call.
 */
void
cpu_force_thread_destroyall(THREAD *thp) {
	void		*kc_addr;

	MIPS_REG_SETx64(&thp->reg, MIPS_REG_V0, __KER_THREAD_DESTROYALL);
#if defined(VARIANT_r3k)
	#define IS_KERNEL_MODE(sreg) (((sreg) & MIPS3K_SREG_KUp) == 0)
#else
	#define IS_KERNEL_MODE(sreg) (((sreg) & MIPS_SREG_KSU) == MIPS_SREG_MODE_KERNEL)
#endif
	if(IS_KERNEL_MODE(thp->reg.regs[MIPS_CREG(MIPS_REG_SREG)])) {
		/*
		 * If it's a kernel mode thread, use the kseg0 address of the syspage
		 * in case we've already destroyed the address space of the
		 * process.
		 */
		kc_addr = sys_kercallptr;
	} else {
		kc_addr = kercallptr;
	}
	SETKIP(thp, kc_addr);
}

/*
 * CPU specific code to cause a thread to invoke a user function.
 */
void
cpu_invoke_func(THREAD *thp, uintptr_t addr) {
	SETKIP(thp, addr);
	MIPS_REG_SETx64(&thp->reg, MIPS_REG_T9, REGIP(&thp->reg));	
}

__SRCVERSION("cpu_misc.c $Rev: 161879 $");
