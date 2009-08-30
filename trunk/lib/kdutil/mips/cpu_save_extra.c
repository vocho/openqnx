/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <kernel/nto.h>
#include "kdintl.h"

extern unsigned discover_num_tlb(unsigned);
extern void		r3k_save_extra(struct cpu_extra_state *);
extern void		r3k_restore_extra(struct cpu_extra_state *);
extern void		mips32_save_extra(struct cpu_extra_state *);
extern void		mips64_save_extra(struct cpu_extra_state *);
extern void		mips64_restore_extra(struct cpu_extra_state *);
extern void		sb1_save_extra(struct cpu_extra_state *);

unsigned 		mips_num_tlbs;
static int		is_r3k;
static int		is_sb1=0;
#define HAVE_WIRED() ((__cpu_flags & MIPS_CPU_FLAG_NO_WIRED) == 0)


void
cpu_init_extra(struct cpu_extra_state *state) {
	unsigned cpu_prid = getcp0_prid();	
	//If there's no wired register, it's an R3K
	is_r3k = __cpu_flags & MIPS_CPU_FLAG_NO_WIRED;

	if(MIPS_PRID_COMPANY(cpu_prid) != 0) {
		unsigned	cfg1 = CP0REG_SEL_GET(16,1);

		mips_num_tlbs = 1 +
			((cfg1 & MIPS_CONFIG1_MMUSIZE_MASK) >> MIPS_CONFIG1_MMUSIZE_SHIFT);
	} else {
		mips_num_tlbs = discover_num_tlb(
				is_r3k ? MIPS3K_TLB_INX_INDEXSHIFT : MIPS_TLB_INX_INDEXSHIFT);
	}
	#define MAX_TLB_ROOM	(sizeof(state->tlb)/sizeof(state->tlb[0]))
	if(mips_num_tlbs > MAX_TLB_ROOM) {
		mips_num_tlbs = MAX_TLB_ROOM;
	}
	switch( MIPS_PRID_COMPANY_IMPL_CANONICAL(cpu_prid) ) {
	case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_BROADCOM,MIPS_PRID_IMPL_SB1):
		is_sb1=1;
		break;
	default:
		break;
    }
}


void
cpu_save_extra(struct cpu_extra_state *state) {
	if(is_r3k) {
		r3k_save_extra(state);
	} else if(__cpu_flags & MIPS_CPU_FLAG_64BIT) {
		if (is_sb1) {
			sb1_save_extra(state);
		}
		else {
			mips64_save_extra(state);
		}
	} else {
		mips32_save_extra(state);
	}
}


void
cpu_restore_extra(struct cpu_extra_state *state) {
	// We might have fiddled TLB entries while the kernel
	// debugger was running, so restore things to their
	// original state.
	if(mmu_needs_restoring) {
		mmu_needs_restoring = 0;
		if(is_r3k) {
			r3k_restore_extra(state);
		} else {
			mips64_restore_extra(state);
		}
	}
	CP0REG_SET(10, state->regs.entryhi);
	CP0REG_SET(2, state->regs.entrylo0);
	CP0REG_SET(0, state->regs.index);
	if(!is_r3k) {
		CP0REG_SET(5, state->regs.pagemask);
		CP0REG_SET(2, state->regs.entrylo1);
	}
}
