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
 * Note: this file is an override of the generic one for performance
 * reasons. It is really unfortunate we have this duplicated code, but
 * the generic code caused us to be slower in various mutex/sem benchmarks
 * due to the abstractions and extra function call/parameters. We should
 * revisit and try to optimize/recode in a cpu-independent fashion.
 */

OBJECT *
vmm_vaddr_to_memobj(PROCESS *prp, void *addr, unsigned *offset, int mark_page) {
	pte_t			*ptep;
	uint32_t		pte_lo;
	unsigned		pgsize;
	paddr_t			paddr;
	uintptr_t		vaddr = (uintptr_t)addr;
	ADDRESS			*adp;

	if((vaddr >= MIPS_R4K_K0BASE) && (vaddr < (MIPS_R4K_K1BASE+MIPS_R4K_K1SIZE))) {
		// Assuming kseg0 & kseg1 are the same size
		paddr = vaddr & (MIPS_R4K_K0SIZE - 1);
		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

	CRASHCHECK(prp == NULL);
	adp = prp->memory;
	CRASHCHECK(adp == NULL);

	ptep = adp->cpu.pgdir[L1IDX(vaddr)];

	if(ptep != NULL) {
		ptep += L2IDX(vaddr);
		pte_lo = ptep->lo;
		if(PTE_PRESENT(pte_lo)) {
			pgsize = PGMASK_TO_PGSIZE(ptep->pm);
			if(pgsize > __PAGESIZE) {
				// Because we scramble the PTE's up when dealing with
				// big pages so that the TLB miss handler can be quick,
				// the ptep that we got up above might not have the correct
				// paddr information in it for this address. Because of the
				// way cpu_pte_merge() works, we can get the PTE for the
				// start of big page and artificially multiply the page
				// size by 2. Then when we pull the paddr out of the PTE
				// everything works out.
				uintptr_t check = vaddr & ~(pgsize*2 - 1);

				pte_lo = adp->cpu.pgdir[L1IDX(check)][L2IDX(check)].lo;
				pgsize *= 2;
			}

			paddr = PTE_PADDR(pte_lo) + (vaddr & (pgsize - 1));

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
	}
	CRASHCHECK(1);
	return (OBJECT *)-1;
}

__SRCVERSION("vmm_vaddr_to_memobj.c $Rev: 153052 $");
