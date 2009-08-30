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
#include <arm/opcode.h>
#include <arm/mmu.h>

static uintptr_t
next_instruction(CPU_REGISTERS *ctx)
{
	unsigned	pc  = ctx->gpr[ARM_REG_PC];
	unsigned	ps  = ctx->spsr;
	unsigned	op  = *(unsigned *)pc;
	int			ex  = 0;

	if (ARM_IS_BRANCH(op)) {
		/*
		 * Test branch condition
		 */
		switch (ARM_COND(op)) {
		case ARM_COND_EQ:
			ex = (ps & ARM_CPSR_Z) != 0;
			break;
		case ARM_COND_NE:
			ex = ((ps & ARM_CPSR_Z) == 0);
			break;
		case ARM_COND_CS:
			ex = ((ps & ARM_CPSR_C) != 0);
			break;
		case ARM_COND_CC:
			ex = ((ps & ARM_CPSR_C) == 0);
			break;
		case ARM_COND_MI:
			ex = ((ps & ARM_CPSR_N) != 0);
			break;
		case ARM_COND_PL:
			ex = ((ps & ARM_CPSR_N) == 0);
			break;
		case ARM_COND_VS:
			ex = ((ps & ARM_CPSR_V) != 0);
			break;
		case ARM_COND_VC:
			ex = ((ps & ARM_CPSR_V) == 0);
			break;
		case ARM_COND_HI:
			ex = ((ps & ARM_CPSR_C) == 0 && (ps & ARM_CPSR_Z) == 0);
			break;
		case ARM_COND_LS:
			ex = ((ps & ARM_CPSR_C) == 0 || (ps & ARM_CPSR_Z) != 0);
			break;
		case ARM_COND_GE:
			ex = ((ps & ARM_CPSR_N) == (ps & ARM_CPSR_V));
			break;
		case ARM_COND_LT:
			ex = ((ps & ARM_CPSR_N) != (ps & ARM_CPSR_V));
			break;
		case ARM_COND_GT:
			ex = ((ps & ARM_CPSR_Z) == 0 && (ps & ARM_CPSR_N) == (ps & ARM_CPSR_V));
			break;
		case ARM_COND_LE:
			ex = ((ps & ARM_CPSR_Z) != 0 && (ps & ARM_CPSR_N) != (ps & ARM_CPSR_V));
			break;
		case ARM_COND_AL:
			ex = 1;
			break;
		case ARM_COND_NV:
			ex = 0;
			break;
		default: break;
		}
	}

	/*
	 * If condition passed, target is specified by 24-bit signed offset.
	 * Otherwise, go to next instruction
	 */
	return pc + (ex ? (((int)(op << 8) >> 6) + 8) : 4);
}

static void
code_modify(BREAKPT *d, unsigned new) {
	unsigned	*addr = (unsigned *)d->brk.addr;

#ifdef	VARIANT_v6
	/*
	 * On ARMv6, user read-only pages are also read-only to kernel access.
	 * Create a temporary mapping to the breakpoint address to modify the
	 * instruction.
	 */
	{
		static pte_t	pte_prot;
		pte_t			*pte;
		unsigned		*alias;

		if (pte_prot == 0) {
			struct arm_cpu_entry	*arm_cpu = SYSPAGE_CPU_ENTRY(arm, cpu);

			pte_prot = arm_cpu->kpte_rw;
		}

		/*
		 * Map the physical address at ARM_V6_SCRATCH_BKPT.
		 */
		alias = (unsigned *)(ARM_V6_SCRATCH_BKPT + ((unsigned)addr & PGMASK));
		pte = VTOPTEP(alias);
		*pte = (d->cpu.paddr & ~PGMASK) | pte_prot;
		arm_v4_dtlb_addr((unsigned)alias & ~PGMASK);
		arm_v6_dsb();
		*alias = new;
		__asm__ __volatile__("mcr p15, 0, %0, c7, c14, 1" : : "r" (alias));
		*pte = 0;
		arm_v4_dtlb_addr((unsigned)alias & ~PGMASK);
	}
#else
	/*
	 * WARNING: assumes kernel can write to RO user mappings without faulting.
	 */
	*addr = new;

	/*
	 * Most ARMv4 cpus require the MVA for cache flush operations.
	 *
	 * The PID register is valid here because we are only called:
	 *	- within running process during faults
	 *	- after calling memmgr.aspace during context switches
	 *
	 * This should be harmless for processor cores that automatically
	 * modify the VA supplied to the CP15 cache operations.
	 */
	addr = (void *)((uintptr_t)addr | arm_v4_fcse_get());
#endif
	CacheControl(addr, 4, MS_INVALIDATE_ICACHE);
}

static void
break_set(BREAKPT *brk) {
	paddr_t		paddr;
	
	/*
	 * Silently ignore breakpoint if memory is not mapped.
	 */
	if(memmgr.vaddrinfo(aspaces_prp[KERNCPU], brk->brk.addr, &paddr, NULL, VI_PGTBL) == PROT_NONE) {
		return;
	}
	if (brk->planted++ == 0) {
	
		brk->cpu.org_val = *(unsigned *)brk->brk.addr;
#ifdef	VARIANT_v6
		brk->cpu.paddr = paddr;
#endif
		code_modify(brk, OPCODE_BREAK);
	}
}

static void
break_clr(BREAKPT *brk) {
	if(brk->planted) {
		if (--brk->planted == 0) {
			code_modify(brk, brk->cpu.org_val);
		}
	}
}

int rdecl
cpu_debug_sstep(DEBUG *dep, THREAD *thp)
{
	return EOK;
}

int rdecl
cpu_debug_fault(DEBUG *dep, THREAD *thp, siginfo_t *info, unsigned *pflags)
{
	if (info->si_fltno == FLTBPT) {
		if (dep->cpu.step.planted && KIP(thp) == dep->cpu.step.brk.addr) {
			info->si_fltno = FLTTRACE;
			*pflags |= _DEBUG_FLAG_SSTEP;
			thp->flags &= ~_NTO_TF_SSTEP;
			break_clr(&dep->cpu.step);
			return 0;
		}
		*pflags |= _DEBUG_FLAG_TRACE_EXEC;
	}
	return 0;
}


int rdecl
cpu_debug_brkpt(DEBUG *dep, BREAKPT *bpp)
{
	if (bpp->brk.size == -1) {
		return EOK;
	}

	/*
	 * We only support execution breakpoints
	 */
	if (bpp->brk.type == _DEBUG_BREAK_EXEC) {
		if (bpp->brk.size == 0 && (bpp->brk.addr & 3) == 0) {
			return EOK;
		}
	}
	return EINVAL;
}

void rdecl
cpu_debug_attach_brkpts(DEBUG *dep)
{
	BREAKPT	*d;
	THREAD	*act;

	for (d = dep->brk; d; d = d->next) {
		if (d != dep->skip_brk) {
			break_set(d);
		}
	}

	/*
	 * Check if single-step breakpoints need to be added
	 */
	act = actives[KERNCPU];
	if (act->flags & _NTO_TF_SSTEP) {
		dep->cpu.step.brk.addr = next_instruction(&act->reg);
		break_set(&dep->cpu.step);
	}
}

void rdecl
cpu_debug_detach_brkpts(DEBUG *dep)
{
	BREAKPT	*d;

	if (dep->cpu.step.planted) {
		break_clr(&dep->cpu.step);
	}

	for (d = dep->brk; d; d = d->next) {
		if (d != dep->skip_brk) {
			break_clr(d);
		}
	}
}

int rdecl
cpu_debug_get_altregs(THREAD *thp, debug_altreg_t *reg)
{
	return EINVAL;
}

int rdecl
cpu_debug_set_altregs(THREAD *thp, debug_altreg_t *reg)
{
	return EINVAL;
}

__SRCVERSION("cpu_debug.c $Rev: 153052 $");
