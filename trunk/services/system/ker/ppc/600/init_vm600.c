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
#include <ppc/603cpu.h>
#include <ppc/700cpu.h>

extern const struct exc_copy_block mmu_on;
extern const struct exc_copy_block mmu_off;

extern const struct exc_copy_block  __exc_itlb_603;
extern const struct exc_copy_block  __exc_dtlbr_603;
extern const struct exc_copy_block  __exc_dtlbw_603;

extern const struct exc_copy_block  __exc_itlb_755;
extern const struct exc_copy_block  __exc_dtlbr_755;
extern const struct exc_copy_block  __exc_dtlbw_755;


static const struct trap_entry traps_tlb603[] = {
	{PPC603_EXC_ITMISS,	 NULL, &__exc_itlb_603},
	{PPC603_EXC_DLTMISS, NULL, &__exc_dtlbr_603},
	{PPC603_EXC_DSTMISS, NULL, &__exc_dtlbw_603},
};

static const struct trap_entry traps_tlb755[] = {
	{PPC603_EXC_ITMISS,	 NULL, &__exc_itlb_755},
	{PPC603_EXC_DLTMISS, NULL, &__exc_dtlbr_755},
	{PPC603_EXC_DSTMISS, NULL, &__exc_dtlbw_755},
};

void
get_mmu_code(const struct exc_copy_block **on, const struct exc_copy_block **off) {
	*on  = &mmu_on;
	*off = &mmu_off;
}

void
copy_vm_code() {
	if(!(__cpu_flags & PPC_CPU_HW_HT)) {

		// We can either have a 603e or 755 when we get here. Unfortunately,
		// the 755 is slightly different from the 603e in that it doesn't
		// have the "shadow" registers, so we need different exception code.

		if(__cpu_flags & PPC_CPU_TLB_SHADOW) {
			trap_install_set(traps_tlb603, NUM_ELTS(traps_tlb603));
		} else {
			trap_install_set(traps_tlb755, NUM_ELTS(traps_tlb755));
		}
	}
}

__SRCVERSION("init_vm600.c $Rev: 153052 $");
