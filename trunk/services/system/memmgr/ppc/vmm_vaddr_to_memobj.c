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
	pte_t			**pgdir;
	pte_t			*pde;
	pte_t			*ptep;
	paddr_t			paddr;
	uintptr_t		vaddr = (uintptr_t)addr;

	if(vaddr < VM_KERN_LOW_SIZE) {
		paddr = vaddr;
		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

	if(vaddr >= VM_SYSPAGE_ADDR && vaddr < (VM_SYSPAGE_ADDR+__PAGESIZE)) {
		paddr = (uintptr_t)_syspage_ptr + (vaddr - VM_SYSPAGE_ADDR);

		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

	if(vaddr >= VM_CPUPAGE_ADDR && vaddr < (VM_CPUPAGE_ADDR+__PAGESIZE)) {
		paddr = (uintptr_t)_cpupage_ptr + (vaddr - VM_CPUPAGE_ADDR);

		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

	pgdir = prp->memory->cpu.pgdir;

	pde = PDE_ADDR(pgdir[L1PAGEIDX(vaddr)]);
	if(pde == NULL) goto fail;
	ptep = &pde[L2PAGEIDX(vaddr)];
	if(!PTE_PRESENT(ptep)) goto fail;
	paddr = PTE_PADDR(ptep) + (vaddr & (PTE_PGSIZE(ptep) - 1));

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
	CRASHCHECK(1);
	return (OBJECT *)-1;
}

__SRCVERSION("vmm_vaddr_to_memobj.c $Rev: 153052 $");
