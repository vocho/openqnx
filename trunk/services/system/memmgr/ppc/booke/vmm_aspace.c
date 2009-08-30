/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. All Rights Reserved.
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

#include "vmm.h"

#ifdef	VARIANT_smp

extern void	wire_sync(ADDRESS *);

intrspin_t	asid_lock;

static void
smp_sync_tlb(ADDRESS *adp)
{
	unsigned	my_cpu = 1 << RUNCPU;

	if (adp->cpu.pending_asid_flush & my_cpu) {
		atomic_clr(&adp->cpu.pending_asid_flush, my_cpu);
		tlbop.flush_asid(adp->cpu.asid);
	}
	if (adp->cpu.pending_wire_sync & my_cpu) {
		atomic_clr(&adp->cpu.pending_wire_sync, my_cpu);
		wire_sync(adp);
	}
}
#else

#define smp_sync_tlb(adp)

#endif

void
vmm_aspace(PROCESS *actprp, PROCESS **pactprp) {
	ADDRESS					*adp;

	if((adp = actprp->memory)) {
		InterruptDisable();
		SPINLOCK(&asid_lock);
		if(adp->cpu.asid == PPC_INVALID_ASID) {
			//RUSH3: later move it out for minimize int disable time
			fam_pte_asid_alloc(adp);
		}
		SPINUNLOCK(&asid_lock);
		smp_sync_tlb(adp);
		set_l1pagetable(adp->cpu.pgdir, adp->cpu.asid);
		*pactprp = actprp;
		InterruptEnable();
	}
}

__SRCVERSION("vmm_aspace.c $Rev: 152112 $");
