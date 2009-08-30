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

/**********************************************************************************
 * NOTE - that this file overrides services/system/ker/ppc/cpu_debug.c
 *
 * This is because it requires different handling for the DE bit and this
 * support is still experimental.
 *
 * Rather than break debugging on other chips we are putting it in here for now.
 **********************************************************************************/

#include "externs.h"

#if defined(VARIANT_600) || defined(VARIANT_800) || defined(VARIANT_900)
	#define SSTEP_MSR_FLAGS (PPC_MSR_DE | PPC_MSR_SE)
#elif defined(VARIANT_booke)
	#undef SSTEP_MSR_FLAGS
#elif defined(VARIANT_400)
	#undef SSTEP_MSR_FLAGS 
#else
	#error not configured for processor family
#endif

extern uintptr_t next_instruction(CPU_REGISTERS *ctx);

extern void ppc_set_watch(DEBUG *dep, BREAKPT *bpp);
extern void ppc_clear_watch(DEBUG *dep, BREAKPT *bpp);
extern BREAKPT  *ppc_identify_watch(DEBUG *dep, THREAD *thp, unsigned *pflags);
extern int  ppc_can_watch(DEBUG *dep, BREAKPT *bpp);

/*
 * This routine does enables debugging on a thread. The thread
 * flag _NTO_TF_SSTEP is also set on this thread, so this could
 * be used by the fault handler code if needed. If single stepping
 * is done throught temporary breakpoints, the temp information could
 * be stored in the cpu area off the DEBUG structure so the 
 * cpu_debug_attach_brkpts() function can install them when the
 * thread is being executed.
 * On entry:
 *   dep
 *      DEBUG structure attached to the process being debugged
 *   thp
 *      thread to debug (_NTO_TF_SSTEP) will be set if function succeeds
 * On Exit:
 *   a errno is returned, single step will only occur if EOK is returned.
 */
int rdecl
cpu_debug_sstep(DEBUG *dep, THREAD *thp) {
//kprintf("SS from %x\n", KIP(thp));
	return EOK;
}

/*
 * This routine is called when adding or removing breakpoints from the
 * process debug list. It allows selecting of the type of breakpoints
 * and watchpoints supported by the cpu. The breakpoints are not actually
 * written to the address space during this function, but when the process
 * is switched to via cpu_debug_attach_brkpts(). It will only be called
 * after cpu_debug_detach_brkpts() has been called, so it does not need to
 * work about this case.
 * On entry:
 *   dep
 *      DEBUG structure of process for breakpoints.
 *   bpp
 *      BREAKPT structure with breakpoint to install.
 *      
 * On exit:
 *   a errno is returned, the breakpoint will not be added unless EOK is returned.
 * 
 */
 /* PPC only support the following watch points currently:
 		_MODIFY
		No HW supported
 */
int rdecl
cpu_debug_brkpt(DEBUG *dep, BREAKPT *bpp) {
	size_t							size;

	if(bpp->brk.size == -1) {
		return EOK;
	}

	switch(bpp->brk.type) {
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_MODIFY:
		if(		bpp->brk.size > sizeof bpp->cpu.data.value ||
				(size = memmgr.mapinfo(dep->process, bpp->brk.addr, NULL, NULL, 0, NULL, NULL, NULL)) == 0 ||
				bpp->brk.size > size) {
			return EINVAL;
		}

		// Assumes that the debugee process has it's address
		// space active, which pathmgr/procfs.c seems to be ensuring.

		memcpy(bpp->cpu.data.value, (void*)bpp->brk.addr, bpp->brk.size);

		/* Fallthrough */
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_RD:
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_WR:
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_RW:
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_RWM:
		if ( ppc_can_watch(dep, bpp) ) {
			return EOK;
		}
		break;

	case _DEBUG_BREAK_EXEC:
		if(bpp->brk.size == 0) {
			break;
		}
		/* Fall Through */
	default:
		return EINVAL;
	}
	return EOK;
}


static void
code_modify(BREAKPT *d, unsigned new) {
	uintptr_t	vaddr = d->brk.addr;

#if defined(VARIANT_600)
	/*
	 * On the 600 series, we cannot use physical address as
	 * we do on other processors, so instead we do some segment
	 * register magic to write breakpoints. We make the segment writable
	 * by overriding the Ks key in the segment register.
	 */
	uint32_t	sreg;

	/* Segment register twiddling must happen with interrupts off */
	InterruptDisable();
	ppc_sync();
	sreg = get_sreg(vaddr);
	set_sreg(vaddr, sreg & ~_ONEBIT32B(1));
	ppc_isync();
	*(uint32_t *)vaddr = new;
	ppc_sync();
	set_sreg(vaddr, sreg);
	ppc_isync();
	InterruptEnable();
#elif defined(VARIANT_900)
	/*
	 * On the 900 series, we cannot use physical address as
	 * we do on other processors, so instead we do some segment
	 * register magic to write breakpoints. We make the segment writable
	 * by overriding the Ks key in the SLB entry
	 */
	unsigned	v,e;
	unsigned	idx;

	/* Segment register twiddling must happen with interrupts off */
	InterruptDisable();
	idx = vaddr >> 28;
	e = (idx << PPC64_SLB1_ESID_SHIFT) | idx | PPC64_SLB1_V;
	ppc_slbmfev(v, idx);
	ppc_isync();
	ppc_slbie(vaddr & 0xf0000000);
	ppc_slbmte(v & ~PPC64_SLB0_KS, e);

	//RUSH3: This next line shouldn't be required, but for some
	//RUSH3: reason the 970FX hangs on the store without it (on a
	//RUSH3: read of the vaddr location as well). It introduces
	//RUSH3: a dependency on the chip having the optional Bridge to
	//RUSH3: SLB functionality, which we don't have anywhere else :-(.
	//RUSH3: Something to investigate if we ever get a JTAG debugger
	//RUSH3: and can see what the 970FX is doing on the store (doesn't
	//RUSH3: seem to be taking any exception).
	asm(".cpu ppc64bridge"); set_sreg(vaddr,get_sreg(vaddr));

	ppc_isync();
	*(uint32_t *)vaddr = new;
	ppc_isync();
	ppc_slbie(vaddr & 0xf0000000);
	ppc_slbmte(v, e);
	ppc_isync();
	InterruptEnable();
#elif defined(VARIANT_booke)
	ppcbke_tlb_t	tlb;

	tlb.rpn = d->cpu.paddr & ~(__PAGESIZE-1);
	tlb.epn = vaddr & ~(__PAGESIZE-1);
	tlb.tid = get_spr(PPCBKE_SPR_PID);
	tlb.attr = PPCBKE_TLB_ATTR_M;
	tlb.access = PPCBKE_TLB_ACCESS_SR|PPCBKE_TLB_ACCESS_SW;
	tlb.size = PPCBKE_TLB_SIZE_4K;
	tlb.v = 1;
	tlb.ts = 1;
	InterruptDisable();
	tlbop.flush_vaddr(vaddr, get_spr(PPCBKE_SPR_PID));

	// This TLBSYNC is here because the E500 has a timing issue where if
	// the bus is busy (DMA/interrupts), it appears the broadcast of the
	// TLB invalidate from the flush_vaddr() call above can be delayed
	// until after the write_entry() below and the chip either doesn't
	// recognize that it was the one that did the broadcast, or something
	// else in the system reflects the broadcast back at the chip. In 
	// any account, it turns around and invalidates the entry that we
	// just wrote below.
	asm volatile("tlbsync");

	tlbop.write_entry(-1, -1, &tlb);
	ppc_isync();
	*(uint32_t *)vaddr = new;
	ppc_isync();
	tlbop.flush_vaddr(vaddr, get_spr(PPCBKE_SPR_PID));
	InterruptEnable();
#elif defined(VARIANT_400) | defined(VARIANT_800)

	paddr_t 	paddr = d->cpu.paddr;

	//NYI: Doesn't handle stuff above 1G line.
	*(uint32_t *)paddr = new;
	vaddr = paddr; // for cache flush...
#else
	#error not configured for system
#endif

	//NYI: This is a potential bug on an SMP system with non-coherent
	//icaches (PPC700 series). The icache flush will only happen on this
	//one processor. If the other has a copy of the cache line in it already,
	//it could end up missing the breakpoint. Since only one company
	//wanted PPC700 SMP, and they're not using it anymore, we can avoid
	//actually fixing it for right now and just keep it in mind if the
	//issue comes up again (suggest always doing the icache flush whether
	//the code is modified or not).
	icache_flush(vaddr);
}

/*
 * Plant one breakpoint.
 */
static void
one_break(BREAKPT *d) {
	if(memmgr.vaddrinfo(aspaces_prp[KERNCPU], d->brk.addr, &d->cpu.paddr, NULL, VI_PGTBL) != PROT_NONE) {
		if(d->planted++ == 0) {
			d->cpu.data.ins = *(uint32_t *)d->brk.addr;
			code_modify(d, OPCODE_BREAK);
		}
	}
}

/*
 * This routine is called at process switch time from the kernel.
 * It is responsible for modifying the processes address space to
 * plant breakpoints. It needs to take care if the region is not
 * mapped writable, probably calling memmgr.vaddrinfo() to get
 * physical memory that it can write to. If there is no physical
 * memory there (i.e. the page hasn't been faulted in yet) it should
 * skip this breakpoint quietly. When the page is faulted in, it
 * will get another chance to plant the breakpoint and it can't
 * be referenced untill it is faulted in anyhow.
 * On entry:
 *   dep
 *      DEBUG structure of process with breakpoint list.
 */
void rdecl
cpu_debug_attach_brkpts(DEBUG *dep) {
	BREAKPT		*d;
	THREAD		*act;
	int			step = 0;
	int			trigger = 0;

	act = actives[KERNCPU];

	if((act->flags & _NTO_TF_SSTEP) && !(act->internal_flags & _NTO_ITF_SSTEP_SUSPEND)) {
		step = 1;
#ifdef DEBUG_DEBUG
		kprintf("ab step = 1\n");
#endif
	}
	for(d = dep->brk; d; d = d->next) {
#ifdef DEBUG_DEBUG
		kprintf("nw is %d\n", dep->cpu.num_watchpoints );
#endif
		if(d != dep->skip_brk) {
			if ( d->brk.type & _DEBUG_BREAK_MODIFY ) {
				if ( memcmp(d->cpu.data.value, (void*)d->brk.addr, d->brk.size) ) {
#ifdef DEBUG_DEBUG
					kprintf("trigger! %x %x\n", d->cpu.data.value, (void *)d->brk.addr);
#endif
					trigger = 1;
					break;
				}
			}
			if(d->brk.type & _DEBUG_BREAK_EXEC) {
				one_break(d);
			} else if( !(act->flags & _NTO_TF_SSTEP)) {
				dep->cpu.num_watchpoints++;
				if ( dep->cpu.num_watchpoints >= dep->cpu.max_hw_watchpoints ) {
					step = 1;
					d->cpu.hwreg = ~0;
#ifdef DEBUG_DEBUG
					kprintf("SOFT nw is %d\n", dep->cpu.num_watchpoints );
#endif
				} else {
					d->cpu.hwreg = dep->cpu.num_watchpoints-1; /* hwreg is 0 based */
					ppc_set_watch( dep, d );
#ifdef DEBUG_DEBUG
					kprintf("HARD nw is %d\n", dep->cpu.num_watchpoints );
#endif
				}
			}
		}
	}

#ifdef VARIANT_booke
	if(step) {
		dep->cpu.dbcr0 |= PPCBKE_DBCR0_ICMP;
	}
	set_spr( PPCBKE_SPR_DBCR0, PPCBKE_DBCR0_IDM | dep->cpu.dbcr0 );
	ppc_isync();
#ifdef DEBUG_DEBUG
	kprintf("ab msr %x dbcr0 %x\n", get_msr(), get_spr(PPCBKE_SPR_DBCR0) );
#endif
#else
	if(step) {
#if defined(SSTEP_MSR_FLAGS)
		act->reg.msr |= SSTEP_MSR_FLAGS;
#else
		dep->cpu.step.brk.addr = next_instruction(&act->reg);
		if(dep->cpu.step.brk.addr != -1) {
			one_break(&dep->cpu.step);
		}
#endif
	}
#endif
	if ( trigger ) {
		usr_fault(MAKE_SIGCODE(SIGTRAP,TRAP_TRACE,FLTTRACE), act, 0);
		__ker_exit();
	}
}

/*
 * This routine is called at process switch time from the kernel.
 * It is responsible for modifying the processes address space to
 * remove breakpoints. It needs to take care if the region is not
 * mapped writable, probably calling memmgr.vaddrinfo() to get
 * physical memory that it can write to. It should only remove
 * things that where modified during cpu_debug_attach_brkpts().
 * On entry:
 *   dep
 *      DEBUG structure of process with breakpoint list.
 */
void rdecl
cpu_debug_detach_brkpts(DEBUG *dep) {
	BREAKPT				*d;

	for(d = dep->brk; d; d = d->next) {
		if(d != dep->skip_brk) {
			if(d->planted) {
				if(--d->planted == 0) {
					code_modify(d, d->cpu.data.ins);
				}
			}
#ifdef VARIANT_booke
			if ( d->cpu.hwreg != ~0U ) {
				ppc_clear_watch(dep,d);
			}
#endif
		}
	}

#if VARIANT_booke
#ifdef DEBUG_DEBUG
	kprintf("db msr %x dbcr0 %x\n", get_msr(), get_spr(PPCBKE_SPR_DBCR0) );
#endif
	set_spr( PPCBKE_SPR_DBCR0, 0 );
	ppc_isync();
#elif !defined(SSTEP_MSR_FLAGS)
	if(dep->cpu.step.planted) {
		dep->cpu.step.planted = 0;
		code_modify(&dep->cpu.step, dep->cpu.step.cpu.data.ins);
	}
#endif
	dep->cpu.num_watchpoints = 0;
}

/*
 * This routine is called to allow cpu specific modifying and identifying
 * during a fault. If it is reporting a fault, it must turn off any
 * single step that was enabled by cpu_debug_sstep. If needed, this routine
 * could scan "MODIFY" breakpoints (if they aren't done with hardware) to
 * see if they changed.
 * On entry:
 *   dep
 *      DEBUG structure attached to the process (for cpu specific information)
 *   thp
 *      thread that had the fault (KIP(thp) is the ip that had the faulting instruction)
 *   psigno
 *      Pointer to signal number (This can be modified if nessessary)
 *   psigcode
 *      Pointer to signal code (This can be modified if nessessary)
 *   pfault
 *      Pointer to fault code (This can be modified if nessessary)
 *   pflags
 *      Pointer to debug flags (_DEBUG_FLAG_*). If a data watchpoint was
 *      triggered, the corresponding _DEBUG_FLAG_TRACE_* flag should be set.
 *      If it was an execution breakpoint, _DEBUG_FLAG_ISTOP should be set.
 *      If it was a single step fault, _DEBUG_FLAGS_SSTEP should be set.
 * On Exit (non-zero):
 *   A non-zero return value, causes the fault to be ignored, and the thread
 *   will try to restart at the faulting instruction. This could be used to
 *   make watchpoints by having all the threads in a process single step, then
 *   check after each instruction for the modified memory. If nothing is modified,
 *   continue the thread so the single step occurs after the next instruction.
 * On Exit (zero):
 *   psigno
 *   psigcode
 *   psigfault
 *       These should be modified if nessessary to reflect the real values.
 *   pflags
 *       This should have all _DEBUG_FLAGS set that this fault matches
 */
int rdecl
cpu_debug_fault(DEBUG *dep, THREAD *thp, siginfo_t *info, unsigned *pflags) {
	BREAKPT	*watch = NULL;
	int		state = 0;
	#define WAS_STEPPING 0x1
	#define WAS_WATCHING 0x2

#ifdef DEBUG_DEBUG
kprintf("CPU %d fault %d at %x MSR %x DBCR0 %x DBSR %x\n", KERNCPU, info->si_fltno, KIP(thp), thp->reg.msr, get_spr( PPCBKE_SPR_DBCR0 ), thp->cpu.dbsr );
#endif

	if(info->si_fltno == FLTBPT) {
#if !defined(SSTEP_MSR_FLAGS) && !defined(VARIANT_booke)
		if(dep->cpu.step.planted) {
			if(KIP(thp) == dep->cpu.step.brk.addr) {
				info->si_fltno = FLTTRACE;
			}
		}
#endif
	}

	if(info->si_fltno == FLTTRACE) {

		watch = ppc_identify_watch( dep, thp, pflags );
		if ( watch != NULL ) {
#ifdef DEBUG_DEBUG
			kprintf("found watch for %x\n", watch->brk.addr );
			kprintf("dbsr %x\n", thp->cpu.dbsr);
#endif
			ppc_clear_watch( dep, watch );
			state = WAS_WATCHING;
#ifdef DEBUG_DEBUG
			kprintf("saving real pflags for later\n");
#endif
			dep->cpu.real_step_flags = *pflags | _DEBUG_FLAG_TRACE_MODIFY;
			thp->flags |= _NTO_TF_SSTEP;
			dep->cpu.dbcr0 |= PPCBKE_DBCR0_ICMP;
#ifdef DEBUG_DEBUG
			kprintf("clearing watch register\n");
#endif
			set_spr( PPCBKE_SPR_DBCR0, PPCBKE_DBCR0_IDM | dep->cpu.dbcr0 );
			ppc_isync();

			/* force the new single step breakpoint to be planted */
			cpu_debug_detach_brkpts(dep);
			cpu_debug_attach_brkpts(dep);

			return 1;
		}

		if((thp->flags & _NTO_TF_SSTEP) && !(thp->internal_flags & _NTO_ITF_SSTEP_SUSPEND)) {
			*pflags |= _DEBUG_FLAG_SSTEP;
			thp->flags &= ~_NTO_TF_SSTEP;
#ifdef DEBUG_DEBUG
			kprintf("single step! dbsr %x ICMP = %x\n", thp->cpu.dbsr, PPCBKE_DBSR_ICMP);
#endif
//			set_spr( PPCBKE_SPR_DBSR, ~0 );
#ifdef VARIANT_booke
			dep->cpu.dbcr0 &= ~(PPCBKE_DBCR0_ICMP);
#ifdef DEBUG_DEBUG
			kprintf("removing booke single step! %x dbsr %x\n", dep->cpu.dbcr0, thp->cpu.dbsr );
#endif
			set_spr( PPCBKE_SPR_DBCR0, PPCBKE_DBCR0_IDM | dep->cpu.dbcr0 );
			ppc_isync();
#endif
			if(dep->cpu.real_step_flags) {
#ifdef DEBUG_DEBUG
				kprintf("real step flags were set!\n");
#endif
				*pflags |= dep->cpu.real_step_flags;
				dep->cpu.real_step_flags = 0;
				return 0;
			}
		}
	}
	if(info->si_fltno == FLTBPT) {
		*pflags |= _DEBUG_FLAG_TRACE_EXEC;
	}

#if defined(SSTEP_MSR_FLAGS)
	if(thp->reg.msr & SSTEP_MSR_FLAGS) {
		state |= WAS_STEPPING;
		if(!(state & WAS_WATCHING)) {
			thp->reg.msr &= ~SSTEP_MSR_FLAGS;
#ifdef DEBUG_DEBUG
			kprintf("removing single step!\n");
#endif
		}
	}
#elif !defined(VARIANT_booke)
	if(dep->cpu.step.planted) {
		state |= WAS_STEPPING;

		dep->cpu.step.planted = 0;
		code_modify(&dep->cpu.step, dep->cpu.step.cpu.data.ins);

		if((state & WAS_WATCHING) && (*pflags == 0)) {
			dep->cpu.step.brk.addr = next_instruction(&thp->reg);
			if(dep->cpu.step.brk.addr != -1) {
				one_break(&dep->cpu.step);
			}
		}
	}
#endif

	if((state != 0) && (*pflags == 0)) {
		return 1;
	}

	return 0;
}

int rdecl
cpu_debug_get_altregs(THREAD *thp, debug_altreg_t *reg) {
	if(thp->cpu.alt == NULL) {
		return EINVAL;
	}

	//NYI: SMP
	if(actives_alt[KERNCPU] == thp->cpu.alt) {
		lock_kernel();
		alt_force_save(thp);
		actives_alt[KERNCPU] = NULL;
	}
	memcpy(reg, thp->cpu.alt, alt_souls.size);

	return EOK;
}

int rdecl
cpu_debug_set_altregs(THREAD *thp, debug_altreg_t *reg) {
	if(thp->cpu.alt == NULL) {
		return EINVAL;
	}

	//NYI: SMP
	if(actives_alt[KERNCPU] == thp->cpu.alt) {
		lock_kernel();
		alt_force_save(thp);
		actives_alt[KERNCPU] = NULL;
	}
	memcpy(thp->cpu.alt, reg, alt_souls.size);

	return EOK;
}

__SRCVERSION("$IQ$");
