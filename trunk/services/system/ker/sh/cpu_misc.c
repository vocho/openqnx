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

#undef FILE_DEBUG
/* #define FILE_DEBUG */


// used with walk_asinfo to find the first baseaddr-TMU record
static int find_base_address( struct asinfo_entry* as, char* name, void* variable_addr)
{
    *((paddr_t *)variable_addr) = (paddr_t)as->start;
    return 0;  // Return 0 so we stop searching after the first match.
}

/*
 * CPU specific initialization of syspage pointers & stuff
 */
void
cpu_syspage_init(void) {
	struct cpuinfo_entry	*cpu;
	uint16_t	*kc_addr;
	uint16_t	temp;
	int			i;
	struct cacheattr_entry			*cache;
	int 							num_colours;

	cpu = SYSPAGE_ENTRY(cpuinfo);
	
    // Set cpu features in case startup didn't...
    for (i=0; i<_syspage_ptr->num_cpu; i++) {
		// When we test the cpuinfo->flags field we usually use
		// __cpu_flags which means we are testing CPU 0's flags.  
		// We are assuming that all CPUs share the same flags.  Here
		// we could get away with only setting the flags for cpu[0], but 
		// for completeness we'll set the flag bits on each core.
		if ( (cpu[i].flags & SH_CPU_FLAG_IGNORE_OLD_PVR) == 0 ) {
			
			// If the IGNORE_OLD_PVR flag isn't set, make sure we have a good
			// PVR value.  At least one early startup (6.3.0SP1 biscayne) didn't
			// set the PVR in the cpu field.
			cpu[i].cpu = SH4_PVR_MASK(in32(SH_MMR_PVR));
			
			// If the IGNORE_OLD_PVR flag isn't set we try to set the CPU flags
			// based on the PVR.  If it's not a PVR we recognize we can still
			// set some flags based on the family -- SH4 vs SH4a.  If a processor
			// needs specific flags that aren't covered by the default family settings
			// (e.g. the 7751R needs EMODE) and the processor's PVR isn't known to
			// this code, startup better set the CPU flags explicitly.
			// Note that the IGNORE_OLD_PVR flag was introduced with the 6.4.0 release,
			// so startups built prior to 6.4.0 won't have it set.  Any boards 
			// introduced after 6.4.0 are expected to set it.  Startups created
			// prior to 6.4.0 but compiled after 6.4.0 might have it set, or might not,
			// depending on whether they override init_cpuinfo() or not.
			
			switch ( SH4_PVR_FAM(cpu[i].cpu) ) {
				case SH4_PVR_SH4:
					// SH4 architectures:
					//   o must use code in P2 area for manipulating cache and TLB tables
					//   o must look at TMU base registers through the A7 area
					//   o use special bits in shm_ctl_special
					cpu[i].flags |=     SH_CPU_FLAG_P2_FOR_CACHE
					                  | SH_CPU_FLAG_TMU_THROUGH_A7
					                  | SH_CPU_FLAG_SHM_SPECIAL_BITS;
					/* Note that documentation for shm_ctl_special says that the
					 * special field is only used for SH7760.  But the code has
					 * always supported it for all SH4 processors, so we'll continue
					 * to do that.
					 */
					break;
				case SH4_PVR_SH4A:
					// SH4A architectures:
					//   o support MOVLI/MOVCO
					//   o use 32-bit physical addressing
					cpu[i].flags |=     SH_CPU_FLAG_MOVLICO
					                  | SH_CPU_FLAG_32_BIT;
					break;
				default:
					kprintf("Unsupported CPU family (PVR=0x%x)\n", cpu[i].cpu );
					crash();
			}
	
			/* PCI? */
			if ( (cpu[i].cpu == SH4_PVR_7751R) || 
			     (cpu[i].cpu == SH4_PVR_7751_1_2) || 
			     (cpu[i].cpu == SH4_PVR_7751_3_4_5) ) {
				// These CPUs support PCI.  This list isn't complete, and I don't 
				// believe we use this bit anywhere in QNX-owned code.  I've left
				// it in for legacy support of the SH4_HAS_PCI macro defined in
				// public/sh/7751pci.h.
				cpu[i].flags |= SH_CPU_FLAG_HAS_PCI;
			}
			
			if ( cpu[i].cpu == SH4_PVR_7751R ) {
				cpu[i].flags |= SH_CPU_FLAG_USE_EMODE;
			}
		}
		
	    if (_syspage_ptr->num_cpu > 1) {
	    	cpu[i].flags |= SH_CPU_FLAG_SMP;
	    }
    }
	__cpu_flags = cpu[0].flags;
	 
	// Figure out the cache ways
		    
	cache = &SYSPAGE_ENTRY(cacheattr)[cpu->data_cache];
		
	if (cache->ways == 0) {
		// cache->ways wasn't defined by the startup.  Figure it out the hard way...
		// If it's an SH4, it better be one of the ones we know about.  If it's an
		// SH-4a that we don't know about we assume a 4-way cache.
		// Note that we started filling in cache->ways in startup/lib in the 6.4.0
		// release, so anything 6.4.0 or after is expected to fill in this field.
		switch(cpu->cpu) {
			case SH4_PVR_7750_2_4:
			case SH4_PVR_7750_2_5:
			case SH4_PVR_7750S:
			case SH4_PVR_7751_1_2:
			case SH4_PVR_7751_3_4_5: 	// All these have 1 way cache
				cache->ways = 1;
				break;
					
			case SH4_PVR_7751R:			// 7750R, 7751R, 7760 have 2 way cache
			case SH4_PVR_7760:
				cache->ways = 2;
				break;
					
			case SH4_PVR_7780:			// 7780 has a 4 way cache
			case SH4_PVR_7780_2:		// 7780 has a 4 way cache
			case SH4_PVR_7770:			// 7770 has a 4 way cache
			case SH4_PVR_X3P:			// the SH-4a X3 cores have 4 way caches
				cache->ways = 4;
				break;
				
			default:
				if(SH4_PVR_FAM(cpu->cpu) == SH4_PVR_SH4A) { 	 
					cache->ways = 4;      // sh4a has a 4 way cache 	 
					break; 	 
				}
				// Everything else must be un-supported
				kprintf("No cache data for unsupported CPU (PVR=0x%x)\n", cpu->cpu );
				crash();
		}
	}

	// The CACHE_FLAG_VIRT_IDX is used to distinguish whether or not we need colour
	// support.  It should be set (i.e. indicate virtual indexing) on the CPUs that
	// don't have hardware synonym support.  For the cores that support hardware
	// synonym support, we pretend that they aren't virtually indexed.
	// It is not set correctly for older startups, so we set it here for all non-SMP
	// cores.  For SMP CPUs, we trust what startup tells us.  Note that this means
	// that SMP CPUs run with a single core (i.e. -P1 added to startup) will run
	// with colours.  That will be a bit of a performance hit, but shouldn't impact
	// functionality.  (I don't want to check the PVR for this, since it would mean
	// we would need to touch this code with each new CPU).
	if (_syspage_ptr->num_cpu == 1) {
		// turn on VIRT_IDX
		cache->flags |= CACHE_FLAG_VIRT_IDX;
	}
	
	
	// We need to examine the cache to figure out how many colours we have,
	// so we can adjust the VM_CPUPAGE_ADDR for colour.  We assign the
	// colour_mask_shifted variable (normally done in pa_init called by
	// vmm_init_mem() phase 0, so that the SYSP_ADDCOLOR macro will function
	// correctly.  Note that since we point the colour-adjusted VM_CPUPAGE_ADDR
	// to different cpupages on different cpus, we need to make sure that
	// the cpupages themselves share a colour.  Not an issue on uniprocessor
	// where we only have one cpu page.  It shouldn't be an issue on SMP
	// systems, since hardware support there should ensure that there is
	// only one colour on those systems.  But the 7786 hardware support for
	// colours has a bug, so on that board the startup has to make sure
	// that the different cpupage ptrs have the same colour.
	if ( cache->flags & CACHE_FLAG_VIRT_IDX ) {
		// The startup told us the cache attributes
		num_colours = ((cache->line_size * cache->num_lines)/__PAGESIZE)/cache->ways;
	} else {	
		// startup says we can ignore colours
		num_colours = 1;
	}
	colour_mask_shifted = (num_colours - 1) << 12;
	// With colour_mask_shifted defined, it's now safe to use SYSP_ADDCOLOR and SYSP_GETCOLOR
	
	if(cpu->flags & CPU_FLAG_MMU) {
		privateptr->user_cpupageptr = (void *)SYSP_ADDCOLOR(VM_CPUPAGE_ADDR, SYSP_GETCOLOR(_cpupage_ptr));
		privateptr->user_syspageptr = (void *)SYSP_ADDCOLOR(VM_SYSPAGE_ADDR, SYSP_GETCOLOR(_syspage_ptr));
		for (i = 0; i < _syspage_ptr->num_cpu; i++) {
			cpupageptr[i]->syspage = (void *)SYSP_ADDCOLOR(VM_SYSPAGE_ADDR, SYSP_GETCOLOR(_syspage_ptr));
		}
		kercallptr = (void *)((unsigned)privateptr->user_syspageptr /*VM_SYSPAGE_ADDR*/
				+ ((unsigned)kercallptr - (unsigned)SH_P1_TO_PHYS(_syspage_ptr)));
	} else {
		privateptr->user_cpupageptr = (void *)_cpupage_ptr;
		privateptr->user_syspageptr = (void *)_syspage_ptr;
		kercallptr = (void *)SH_PHYS_TO_P1
				((uintptr_t)kercallptr);
	}


	/*
	 * Set up the code used to force kernel calls.
	 *
	 * Startup has initialised the first two instructions:
	 * kc_addr[0] = trapa
	 * kc_addr[1] = 0xfffd
	 *
	 * We set these up to generate the correct system calls:
	 *
	 * kc_addr[0] = [kercall+0] trapa #__KER_THREAD_DESTROYALL
	 * kc_addr[1] = [kercall+2] 0xfffd
	 *
	 * For SMP, we need to force a __KER_NOP system call, so we use the
	 * 2nd set of instruction locations (not initialised by startup):
	 *
	 * kc_addr[2] = [kercall+4] trapa #__KER_NOP
	 * kc_addr[3] = [kercall+6] 0xfffd
	 */
	kc_addr = (uint16_t *)&SYSPAGE_ENTRY(system_private)->kercall;
	temp = kc_addr[0];
	temp = (temp & 0xff00) | __KER_THREAD_DESTROYALL;
	kc_addr[0] = temp;
	icache_flush(&kc_addr[0]);
#ifdef	VARIANT_smp
	kc_addr[2] = (temp & 0xff00) | __KER_NOP;
	kc_addr[3] = kc_addr[1];
	icache_flush(&kc_addr[2]);
#endif

	/*
	 * Set up per-cpu shadow_imask location:
	 * - for uniprocessor, use _syspage_ptr->un.sh.imask
	 * - for multiprocessor, use _cpupage_ptr->un.sh.imask
	 */
	if (_syspage_ptr->num_cpu > 1) {
		for (i = 0; i < _syspage_ptr->num_cpu; i++) {
			cpupageptr[i]->un.sh.imask = 0;
			__cpu_imask[i] = &cpupageptr[i]->un.sh.imask;
		}
		__shadow_imask = &privateptr->user_cpupageptr->un.sh.imask;
	}
	else {
		_syspage_ptr->un.sh.imask = 0;
		__cpu_imask[0] = &_syspage_ptr->un.sh.imask;
		__shadow_imask = &_syspage_ptr->un.sh.imask;
	}

    /*
     * Note hardware base addresses given to us from startup.  Assign
     * default values as appropriate, in case we're using an older startup
     * that doesn't create the asinfo records.
     */
    sh_mmr_tmu_base_address = SH4_MMR_TMU_BASE_ADDRESS;
    walk_asinfo("baseaddr-TMU", find_base_address, &sh_mmr_tmu_base_address);
}

/*
 * CPU specific stuff for initializing a thread's registers
 */
void
cpu_thread_init(THREAD *act, THREAD *thp, int align) {

#ifdef FILE_DEBUG 
kprintf("CPU thread init. act=%x, thp=%x, pc=%x\n",actives[KERNCPU],thp, thp->reg.pc );
#endif

	thp->reg.sr = SH_SR_FD;

	if(act != NULL) {
		thp->reg.sr |= (act->reg.sr & SH_SR_MD);
	} else {
		/* procmgr thread */
		thp->reg.sr |= SH_SR_MD;
	}
	switch(align) {
	case 0:
	case 1:
		thp->flags |= _NTO_TF_ALIGN_FAULT;
		break;
	default: break;
	}

}

/*
 * CPU specific stuff for putting a thread into execution for the
 * first time. We have to arrange a stack frame such that the function
 * thinks it needs to return to thp->args.wa->exitfunc and that it has
 * been passed thp->args.wa.arg.
 */
void
cpu_thread_waaa(THREAD *thp) {
	thp->reg.pr = (uintptr_t)thp->un.lcl.tls->__exitfunc;
	thp->reg.gr[4] = (uintptr_t)thp->un.lcl.tls->__arg;
}

/*
 * CPU specific stuff for allowing a thread to do priviledged operations (I/O priviledge)
 */
void
cpu_thread_priv(THREAD *thp) {
	/* Nothing needed */
}

/*
 * CPU specific stuff for destroying a thread
 */
void
cpu_thread_destroy(THREAD *thp) {
	/* Nothing needed */
}

/*
 * CPU specific stuff for allowing/disallowing alignment faults on a thread
 */
void
cpu_thread_align_fault(THREAD *thp) {
	/* Nothing needed for SH */
}

/*
 * CPU specific stuff for saving registers on signal delivery
 */
void
cpu_signal_save(SIGSTACK *ssp, THREAD *thp) {
	ucontext_t	*uc;

	uc = ssp->info.context;
	uc->uc_mcontext.cpu.gr[14] 	= thp->reg.gr[14];
	uc->uc_mcontext.cpu.gr[4] 	= thp->reg.gr[4];
	uc->uc_mcontext.cpu.gr[3] 	= thp->reg.gr[3];
	uc->uc_mcontext.cpu.gr[2] 	= thp->reg.gr[2];
	uc->uc_mcontext.cpu.gr[1] 	= thp->reg.gr[1];
	uc->uc_mcontext.cpu.gr[0] 	= thp->reg.gr[0];
	uc->uc_mcontext.cpu.sr		= thp->reg.sr;
}

/*
 * CPU specific stuff for restoring from a signal
 */
void
cpu_signal_restore(THREAD *thp, SIGSTACK *ssp) {
	ucontext_t	*uc;

	uc = ssp->info.context;
	thp->reg.gr[14]	= uc->uc_mcontext.cpu.gr[14];
	thp->reg.gr[4]	= uc->uc_mcontext.cpu.gr[4];
	thp->reg.gr[3]	= uc->uc_mcontext.cpu.gr[3];
	thp->reg.gr[2]	= uc->uc_mcontext.cpu.gr[2];
	thp->reg.gr[1]	= uc->uc_mcontext.cpu.gr[1];
	thp->reg.gr[0] 	= uc->uc_mcontext.cpu.gr[0];
	thp->reg.sr	= uc->uc_mcontext.cpu.sr;
}

/*
 * CPU-specific stuff for setting up the thread environment for transfering
 * to a signal handler through __signalstub.
 *
 * On SH, we need to support pre-6.4.0 libc and binaries where __signalstub
 * is expecting the siginfo pointer in R0.  As of 6.4.0 this old mechanism
 * no longer works, and the 6.4.0 linker generates a __signalstub@plt function
 * that trashes R0 (this is perfectly correct, as the SH ABI says that R0 is
 * fair game to be trashed -- our old __signalstub was breaking the ABI).
 *
 * For 6.4.0 and beyond, __signalstub is expecting the siginfo pointer in R4.
 *
 * So, here, we place the siginfo pointer in both places.
 */
void
sh_setup_sigstub(struct thread_entry* thp, uintptr_t new_sp, uintptr_t siginfo) {
	SETKIP(thp, thp->process->sigstub);
	SETKSP(thp, new_sp);
	SETKSTATUS(thp, siginfo); // siginfo pointer in R0
	thp->reg.gr[4] = siginfo; // siginfo pointer in R4
}

/*
 * CPU specific stuff for saving registers needed for interrupt delivery
 */
void
cpu_intr_attach(INTERRUPT *itp, THREAD *thp) {
	/* Nothing needed for SH */
}

/*
 * This copies the register sets, but makes sure there are no
 * security holes
 */
void cpu_greg_load(THREAD *thp, CPU_REGISTERS *regs) {
	uint32_t		sr = thp->reg.sr;

	thp->reg = *regs;
	/*
	 * Don't restore MSR_PR, it is a security hole
	 */
	thp->reg.sr = (regs->sr & ~SH_SR_MD) | (sr & SH_SR_MD);
}

/*
 * CPU specific stuff for initializing a process's registers
 */
void
cpu_process_startup(THREAD *thp, int forking) {
	if(!forking) {
		thp->reg.sr = SH_SR_FD;

		/* use the mips protocal to pass process startup args */
		/*
		 * Set atexit() pointer to inactive
		 */
		thp->reg.gr[0] = thp->reg.gr[4] = 0;				/* arg atexit */
		thp->reg.gr[5] = thp->reg.gr[15]; 					/* args */

		thp->reg.pr = 0;
	}
}

/*
 * CPU specific stuff for cleaning up before rebooting
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
	if (_syspage_ptr->smp.entry_size > 0) {
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

#ifdef FILE_DEBUG 
kprintf("CPU force thread destroyall. thp=%x,sr=%x\n",thp,thp->reg.sr);
#endif

	if((thp->reg.sr & SH_SR_MD) == 0) {
		SETKIP(thp, kercallptr);
	} else {
		kc_addr = &SYSPAGE_ENTRY(system_private)->kercall;
		SETKIP(thp, kc_addr);
	}
#ifdef FILE_DEBUG 
kprintf("CPU force thread destroyall. addr=%x,inst=%x\n",kc_addr,*(uint16_t*)kc_addr);
#endif
}

void cpu_mutex_adjust(THREAD *act) {
 	
  	if(KSP(act) & 1) {
 		// possible mutex operation
 		uintptr_t ip, up, low;
 
 		ip = KIP(act);
 		low = act->reg.gr[0];
 		up = low + act->reg.gr[1];
 		if(ip>=low && ip<up) {
			if(up - low < 20) {
				//mutex implementation should be short
 				SETKIP(act, low);
			}
 		}
 	}
}
 

__SRCVERSION("cpu_misc.c $Rev: 201493 $");
