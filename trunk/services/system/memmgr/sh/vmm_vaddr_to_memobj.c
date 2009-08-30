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
	uint32_t		*pde;
	uint32_t		pte;
	unsigned		pg_offset;
	paddr_t			paddr;
	uintptr_t		vaddr = (uintptr_t)addr;
	ADDRESS			*adp;
	unsigned		mask;

	/* piece of cake for P1, P2 addresses */
	if(SH_IS_P1(vaddr) || SH_IS_P2(vaddr)) {
		// Assuming P1 & P2 are the same size
		paddr = vaddr & (SH_P1SIZE - 1);
		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

	pg_offset = ADDR_OFFSET(vaddr);

	// Check for system, cpu pages...
	if(ADDR_PAGE(vaddr) == SYSP_ADDCOLOR(VM_CPUPAGE_ADDR, SYSP_GETCOLOR(_cpupage_ptr))) {
		paddr = SH_P1_TO_PHYS((uintptr_t)_cpupage_ptr) | pg_offset;
		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

	if(ADDR_PAGE(vaddr) == SYSP_ADDCOLOR(VM_SYSPAGE_ADDR, SYSP_GETCOLOR(_syspage_ptr))) {
		paddr = SH_P1_TO_PHYS((uintptr_t)_syspage_ptr) | pg_offset;
		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

#ifndef NDEBUG
	if(prp == NULL) crash();
#endif
	adp = prp->memory;
#ifndef NDEBUG
	if(adp == NULL) crash();
#endif

	pde = adp->cpu.pgdir[L1PAGEIDX(vaddr)];

	if((pde == NULL) || !PRESENT((pte = pde[L2PAGEIDX(vaddr)]))) {
		/* not mapped, check for perm mappings */
		goto fail;
	}

	mask = SH_PTE_PGSIZE_MASK(pte);
	paddr = (pte & mask) | (vaddr & ~mask);

	if(mark_page) {
		struct pa_quantum	*pq;
		pq = pa_paddr_to_quantum(paddr);
		if(pq != NULL) {
			pq->flags |= PAQ_FLAG_HAS_SYNC;
		}
	}

	*offset = PADDR_TO_SYNC_OFF(paddr);
	return PADDR_TO_SYNC_OBJ(paddr);

fail:
#ifndef NDEBUG
	crash();
	/* NOTREACHED */
#endif
	return (OBJECT *)-1;
}


__SRCVERSION("vmm_vaddr_to_memobj.c $Rev: 199396 $");
