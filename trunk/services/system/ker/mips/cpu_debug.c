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
#include <mips/cpu.h>
#include <mips/opcode.h>
#include <mips/priv.h>

struct brk_range {
	uintptr_t	lo;
	uintptr_t	hi;
};

/* 
 * MIPS Watchpoint Support
 *
 * MIPS32/64 defines the WatchLo/WatchHi CP0 registers
 *   Broadcom SB1 implements this as two pairs, one for instruction, the other for data watches
 */
static int mips_32_64_can_watch( DEBUG *dep, BREAKPT *bpp )
{
	if ( bpp->brk.type & _DEBUG_BREAK_EXEC ) /* for now */
		return 0;
	if ((bpp->brk.addr & 0x3) != 0) {
		return 0;
	}
	return 1;
}
static void mips_32_64_set_watch( DEBUG *dep, BREAKPT *bpp )
{
unsigned 	watchlo, watchhi;

	watchlo = (bpp->brk.addr & ~0x3);
	if ( (bpp->brk.type & _DEBUG_BREAK_WR) || (bpp->brk.type & _DEBUG_BREAK_MODIFY) )
		watchlo |= 0x1;
	if ( bpp->brk.type & _DEBUG_BREAK_RD )
		watchlo |= 0x2;
		
	watchhi = 1<<30; /* G Bit */

	CP0REG_SET(18, watchlo);
	CP0REG_SET(19, watchhi);
}
static void mips_32_64_clear_watch( DEBUG *dep, BREAKPT *bpp )
{
	CP0REG_SET(18, 0);
	CP0REG_SET(19, 0);
}

static int mips_r7k_can_watch( DEBUG *dep, BREAKPT *bpp )
{
	if ( bpp->brk.type & _DEBUG_BREAK_EXEC ) /* for now */
		return 0;
	if ((bpp->brk.addr & 0x3) != 0) {
		return 0;
	}
	if(memmgr.vaddrinfo(aspaces_prp[KERNCPU], bpp->brk.addr, &bpp->cpu.paddr, NULL, VI_PGTBL) == PROT_NONE) {
		return 0;
	}
	return 1;
}

static void mips_r7k_set_watch( DEBUG *dep, BREAKPT *bpp )
{
uint64_t	watch = 0;

	watch = bpp->cpu.paddr & (0x7fffffffcULL);
	if ( (bpp->brk.type & _DEBUG_BREAK_WR) || (bpp->brk.type & _DEBUG_BREAK_MODIFY) )
		watch |= 0x8000000000000000ULL;
	if ( bpp->brk.type & _DEBUG_BREAK_RD )
		watch |= 0x4000000000000000ULL;
	CP0REG_SEL_SET(18,0,watch);
}
static void mips_r7k_clear_watch( DEBUG *dep, BREAKPT *bpp )
{
	CP0REG_SET(18, 0);
}

static int mips_sb1_can_watch( DEBUG *dep, BREAKPT *bpp )
{
	if ( bpp->brk.type & _DEBUG_BREAK_EXEC ) /* for now */
		return 0;
	if ((bpp->brk.addr & 0x7) != 0) {
		return 0;
	}
	if (bpp->brk.size > 8) {
		return 0;
	}
	return 1;
}

static void mips_sb1_set_watch( DEBUG *dep, BREAKPT *bpp )
{
unsigned 	watchlo, watchhi;

	watchlo = (bpp->brk.addr & ~0x7);

	if ( (bpp->brk.type & _DEBUG_BREAK_WR) || (bpp->brk.type & _DEBUG_BREAK_MODIFY) )
		watchlo |= 0x1;
	if ( bpp->brk.type & _DEBUG_BREAK_RD )
		watchlo |= 0x2;

	watchhi = 1<<30; /* G Bit */

	CP0REG_SEL_SET(18, 1, watchlo); /* Data load/store watch */
	CP0REG_SEL_SET(19, 1, watchhi);
}
static void mips_sb1_clear_watch( DEBUG *dep, BREAKPT *bpp )
{
	CP0REG_SEL_SET(18, 1, 0);
	CP0REG_SEL_SET(19, 1, 0);
}

static int  null_mips_can_watch(DEBUG *dep, BREAKPT *bpp )
{
	return 0;
}

static void null_mips_set_watch( DEBUG *dep, BREAKPT *bpp )
{
}
static void null_mips_clear_watch( DEBUG *dep, BREAKPT *bpp )
{
}

static int select_mips_can_watch(DEBUG *dep, BREAKPT *bpp );
static int (*mips_can_watch)(DEBUG *dep, BREAKPT *bpp ) = select_mips_can_watch;
static void (*mips_set_watch)(DEBUG *dep, BREAKPT *bpp ) = null_mips_set_watch;
static void (*mips_clear_watch)(DEBUG *dep, BREAKPT *bpp ) = null_mips_clear_watch;

static int select_mips_can_watch(DEBUG *dep, BREAKPT *bpp )
{
	unsigned cpu_prid = getcp0_prid();

	switch( MIPS_PRID_COMPANY_IMPL_CANONICAL(cpu_prid) ) {
	case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_MIPS,MIPS_PRID_IMPL_4Kc):
		mips_can_watch = mips_32_64_can_watch;
		mips_set_watch = mips_32_64_set_watch;
		mips_clear_watch = mips_32_64_clear_watch;
		break;
	case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_NONE,MIPS_PRID_IMPL_7000):
		mips_can_watch = mips_r7k_can_watch;
		mips_set_watch = mips_r7k_set_watch;
		mips_clear_watch = mips_r7k_clear_watch;
		break;
	case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_BROADCOM,MIPS_PRID_IMPL_SB1):
		mips_can_watch = mips_sb1_can_watch;
		mips_set_watch = mips_sb1_set_watch;
		mips_clear_watch = mips_sb1_clear_watch;
		break;
	case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_SANDCRAFT,MIPS_PRID_IMPL_SR7100):
		mips_can_watch = mips_32_64_can_watch;
		mips_set_watch = mips_32_64_set_watch;
		mips_clear_watch = mips_32_64_clear_watch;
		break;
	default:
		mips_can_watch = null_mips_can_watch;
		mips_set_watch = null_mips_set_watch;
		mips_clear_watch = null_mips_clear_watch;
		break;
	}
	return mips_can_watch(dep,bpp);
}

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
	if(NUM_PROCESSORS > 1) {
		// tell the upper layer we can't handle a _DEBUG_RUN_STEP_ALL
		return -1;
	}
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
int rdecl
cpu_debug_brkpt(DEBUG *dep, BREAKPT *bpp) {
	size_t		size;

	if(bpp->brk.size == -1) {
		return EOK;
	}

	switch(bpp->brk.type) {
	case _DEBUG_BREAK_EXEC:
		if((bpp->brk.size == 0) && ((bpp->brk.addr & 0x3) == 0)) {
			return EOK;
		}
		break;
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_MODIFY:
		if(		bpp->brk.size > sizeof bpp->cpu.old ||
				(size = memmgr.mapinfo(dep->process, bpp->brk.addr, NULL, NULL, 0, NULL, NULL, NULL)) == 0 ||
				bpp->brk.size > size) {
			return EINVAL;
		}

		// Assumes that the debugee process has it's address
		// space active, which pathmgr/procfs.c seems to be ensuring.

		memcpy(&bpp->cpu.old, (void *)bpp->brk.addr, bpp->brk.size);
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_RD:
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_WR:
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_RW:
	case _DEBUG_BREAK_HW|_DEBUG_BREAK_RWM:
		if ( mips_can_watch(dep, bpp) ) {
			return EOK;
		}
		break;
	default:
		break;
	}
	return EINVAL;
}

static void
update_range(BREAKPT *d, struct brk_range *range) {
	uintptr_t	vaddr = d->brk.addr;

	if(vaddr < range->lo) range->lo = vaddr;
	if(vaddr > range->hi) range->hi = vaddr;
}


//
//KLUDGE: referencing memmgr variables directly. Can't
//link stand alone kernel....
//
extern unsigned	pfn_topshift;
extern unsigned	pt_cacheable_attr;
extern unsigned colour_mask_shifted;

static void
code_modify(BREAKPT *d, struct brk_range *range, unsigned new) {
	uint32_t	lo0;
	uint32_t	lo1;
	uint32_t	hi;
	paddr_t		paddr = d->cpu.paddr;
	uintptr_t	vaddr = d->brk.addr;

	if((paddr < MIPS_R4K_K0SIZE) && (((uintptr_t)paddr & colour_mask_shifted) == (vaddr & colour_mask_shifted))) {
		*(uint32_t *)PHYS_TO_PTR(paddr) = new;
	} else {

		//Make the address temporarily writable.

#if defined(VARIANT_r3k)
		hi = (vaddr & MIPS3K_TLB_HI_VPNMASK) | (getcp0_tlb_hi() & MIPS3K_TLB_HI_ASIDMASK);
		lo0 = (paddr & MIPS3K_TLB_LO_PFNMASK) | (MIPS3K_TLB_VALID | MIPS3K_TLB_WRITE);
		lo1 = 0;
#else 		
		hi = (vaddr & MIPS_TLB_HI_VPN2MASK) | (getcp0_tlb_hi() & MIPS_TLB_HI_ASIDMASK);
		lo0 = ((paddr >> pfn_topshift) & MIPS_TLB_LO_PFNMASK) | pt_cacheable_attr
				| (MIPS_TLB_VALID | MIPS_TLB_WRITE);
		if(vaddr & __PAGESIZE) {
			lo1 = lo0;
			lo0 = 0;
		} else {
			lo1 = 0;
		}
#endif		
		InterruptDisable();
		// Just put it in as a 4K page.
		r4k_update_tlb(hi, lo0, lo1, getcp0_pagemask() & (__PAGESIZE-1));
		*(uint32_t *)vaddr = new;
		(void) r4k_flush_tlb(hi);
		InterruptEnable();
	}
		
	update_range(d, range);
}

/*
 * Plant one breakpoint.
 */
static void
one_break(BREAKPT *d, struct brk_range *range) {

	if(memmgr.vaddrinfo(aspaces_prp[KERNCPU], d->brk.addr, &d->cpu.paddr, NULL, VI_PGTBL) != PROT_NONE) {
		if(d->planted++ == 0) {
			d->cpu.old = *(uint32_t *)d->brk.addr;
			code_modify(d, range, OPCODE_BREAK);
		} else if(noncoherent_caches) {
			update_range(d, range);
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
	struct brk_range range;
	int			trigger = 0;

	range.lo = ~0UL;
	range.hi = 0UL;

	act = actives[KERNCPU];
	for(d = dep->brk; d; d = d->next) {
		if(d != dep->skip_brk) {
			if(d->brk.type & _DEBUG_BREAK_MODIFY) {
				if ( memcmp((void *)d->brk.addr, &d->cpu.old, d->brk.size) != 0 ) {
					trigger = 1;
					break;
				}
			}
			if(d->brk.type & _DEBUG_BREAK_EXEC) {
				one_break(d, &range);
			} else if( !(act->flags & _NTO_TF_SSTEP)) {
				mips_set_watch( dep, d );
			}
		}
	}
	act = actives[KERNCPU];
	if((act->flags & _NTO_TF_SSTEP) && !(act->internal_flags & _NTO_ITF_SSTEP_SUSPEND)) {
		dep->cpu.step.brk.addr = next_instruction(&act->reg);
		one_break(&dep->cpu.step, &range);
		trigger = 0;
	}

	/*
	 * If any breakpoint opcode are written to code area, invalidate Icache.
	 */
	if(range.lo <= range.hi) {
		CacheControl((void *)range.lo, (range.hi - range.lo) + 4, MS_INVALIDATE_ICACHE);
	}

	if ( trigger ) {
		usr_fault(MAKE_SIGCODE(SIGTRAP,TRAP_TRACE,FLTWATCH), act, 0);
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
	struct brk_range	range;

	range.lo = ~0UL;
	range.hi = 0UL;

	if(dep->cpu.step.planted) {
		if(--dep->cpu.step.planted == 0) {
			code_modify(&dep->cpu.step, &range, dep->cpu.step.cpu.old);
		} else if(noncoherent_caches) {
			update_range(&dep->cpu.step, &range);
		}
	}

	for(d = dep->brk; d; d = d->next) {
		if(d != dep->skip_brk) {
			if(d->planted) {
				if(--d->planted == 0) {
					code_modify(d, &range, d->cpu.old);
				} else if(noncoherent_caches) {
					update_range(d, &range);
				}
			}
			if ( d->brk.type & _DEBUG_BREAK_RWM ) {
				mips_clear_watch( dep, d );
			}
		}
	}

	/*
	 * If any breakpoint opcodes are replaced with original one,
	 * invalidate Icache.
	 */
	if(range.lo <= range.hi) {
		CacheControl((void *)range.lo, (range.hi - range.lo) + 4, MS_INVALIDATE_ICACHE);
	}
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
	BREAKPT *d;
	int hwreg;
	unsigned flags = 0;

	switch(info->si_fltno) {
	case FLTBPT:
		if(dep->cpu.step.planted) {
			if(KIP(thp) == dep->cpu.step.brk.addr) {
				*pflags |= _DEBUG_FLAG_SSTEP;
				thp->flags &= ~_NTO_TF_SSTEP;
				if(dep->cpu.real_step_flags) {
					*pflags |= dep->cpu.real_step_flags;
					dep->cpu.real_step_flags = 0;
					return 0;
				}
				info->si_fltno = FLTTRACE;
				return 0;
			}
		}
		// Flag a breakpoint as a TRACE_EXEC
		*pflags |= _DEBUG_FLAG_TRACE_EXEC;
		break;
	case FLTTRACE:
		// This gets used when we finish emulating a FP instruction.
		*pflags |= _DEBUG_FLAG_SSTEP;
		thp->flags &= ~_NTO_TF_SSTEP;
		break;
	case FLTWATCH:
		hwreg = 0; /*mips_hwreg_watch(dep, thp);*/
		mips_clear_watch(dep, NULL);
		for(d = dep->brk; d; d = d->next) {
			if(d != dep->skip_brk) {
				if ( (d->brk.type & _DEBUG_BREAK_RWM) /*&& (d->cpu.hwreg == hwreg)*/) {
					/* NYI
					if (hwreg == 0) {
						flags |= _DEBUG_FLAG_TRACE_WATCH1;
					} else if (hwreg == 1) {
						flags |= _DEBUG_FLAG_TRACE_WATCH2;
					}*/
					if(d->brk.type & _DEBUG_BREAK_MODIFY) {
						if ( memcmp((void *)d->brk.addr, &d->cpu.old, d->brk.size) != 0 ) {
							memcpy(&d->cpu.old, (void *)d->brk.addr, d->brk.size);
							flags |= _DEBUG_FLAG_TRACE_MODIFY;
							break;
						}
					}
					if ( d->brk.type & _DEBUG_BREAK_WR ) {
						flags |= _DEBUG_FLAG_TRACE_WR;
					}
					if ( d->brk.type & _DEBUG_BREAK_RD ) {
						flags |= _DEBUG_FLAG_TRACE_RD;
					}
				}
			}
		}
		/* we need to single step over the instruction that caused
		  the watch exception */
		dep->cpu.real_step_flags = *pflags | flags;
		thp->flags |= _NTO_TF_SSTEP;

		/* force the new single step breakpoint to be planted */
		cpu_debug_detach_brkpts(dep);
		cpu_debug_attach_brkpts(dep);
		return 1;
	default: break;
	}
	return 0;
}

int rdecl
cpu_debug_get_altregs(THREAD *thp, debug_altreg_t *reg) {
#if defined(VARIANT_tx79)

	memcpy(reg, &thp->cpu.alt, sizeof(thp->cpu.alt));

	return EOK;

#else

	return EINVAL;

#endif
}

int rdecl
cpu_debug_set_altregs(THREAD *thp, debug_altreg_t *reg) {

#if defined(VARIANT_tx79)

	memcpy(&thp->cpu.alt, reg, sizeof(thp->cpu.alt));

	return EOK;

#else

	return EINVAL;

#endif
}

__SRCVERSION("cpu_debug.c $Rev: 167933 $");
