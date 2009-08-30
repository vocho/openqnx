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

/*
 * Note: this file is an ovreride of the generic one for performance
 * reasons. It is really unfortunate we have this duplicated code, but
 * the generic code caused us to be slower in various mutex/sem benchmarks
 * due to the abstractions and extra function call/parameters. We should
 * revisit and try to optimize/recode in a cpu-independent fashion.
 */

OBJECT *
vmm_vaddr_to_memobj(PROCESS *prp, void *addr, unsigned *offset, int mark_page) {
	ADDRESS		*adp;
	unsigned	pte;
	unsigned	vaddr = (unsigned)addr;
	unsigned	paddr;

#ifndef NDEBUG
	if (prp == 0) {
		crash();
	}
#endif

	adp = prp->memory;

#if !defined(VARIANT_v6)	
	if (vaddr < USER_SIZE) {
		vaddr |= MVA_BASE(adp->cpu.asid);
	}
#endif	

	/*
	 * Check for section mapping
	 *
	 * Note that bit 4 of the L1 descriptor is implementation defined, so
	 * it may not be set on all cpu implementations.
	 */
	pte = V6_USER_SPACE(vaddr) ? *UTOL1SC(vaddr) : *KTOL1SC(vaddr);
	if ((pte & 0x3) == (ARM_PTP_SC & 3)) {
		paddr = (pte & ~ARM_SCMASK) + (vaddr & ARM_SCMASK);
	}
	else {
		pte = 0;
		if (V6_USER_SPACE(vaddr)) {
			if (*UTOPDIR(vaddr) & ARM_PTE_VALID) {
				pte = *UTOPTEP(vaddr);
			}
		}
		else {
			if (*KTOPDIR(vaddr) & ARM_PTE_VALID) {
				pte = *KTOPTEP(vaddr);
			}
		}
		if (pte == 0) {
#ifndef NDEBUG
			crash();
#endif
			return (OBJECT *)-1;
		}
		if ((pte & ARM_PTE_VALID) == ARM_PTE_LP) {
			paddr = (pte & ~ARM_LPMASK) | (vaddr & ARM_LPMASK);
		}
		else {
			paddr = (pte & ~PGMASK) | (vaddr & PGMASK);
		}
	}

	if(mark_page) {
		struct pa_quantum	*pq;
		pq = pa_paddr_to_quantum(paddr);
		if(pq != NULL) {
			pq->flags |= PAQ_FLAG_HAS_SYNC;
		}
	}

	*offset = PADDR_TO_SYNC_OFF(paddr);
	return PADDR_TO_SYNC_OBJ(paddr);
}

__SRCVERSION("vmm_vaddr_to_memobj.c $Rev: 153052 $");
