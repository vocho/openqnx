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

#include "vmm.h"

// NOTE: This code might be replaced at runtime with the function 
// vm_aspace_bat() in 600/fam_pte.c. Make sure that any changes to 
// this function are reflected there as well.
//
// BookE provides its own implementation to handle SMP asid stealing.
// Make sure that any changes to this function are reflected there as well.
void
vmm_aspace(PROCESS *actprp, PROCESS **pactprp) {
	ADDRESS					*adp;

	if((adp = actprp->memory)) {
		InterruptDisable();
		if(adp->cpu.asid == PPC_INVALID_ASID) {
			//RUSH3: later move it out for minimize int disable time
			fam_pte_asid_alloc(adp);
		}
		set_l1pagetable(adp->cpu.pgdir, adp->cpu.asid
#if defined(VARIANT_600)
				, adp->cpu.nx_state
#endif				
				);
		*pactprp = actprp;
		InterruptEnable();
	}
}

__SRCVERSION("vmm_aspace.c $Rev: 169350 $");
