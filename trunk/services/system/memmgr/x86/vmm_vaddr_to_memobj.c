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
 *
 * We also make the assumption on x86 that the page tables we need are
 * currently active. If this is ever changed in the future, we should hit
 * the crash below and will hae to change the code.
 */
#undef NDEBUG

extern struct cpu_mm_aspace	*pgtbl_list;

OBJECT *
vmm_vaddr_to_memobj(PROCESS *prp, void *addr, unsigned *offset, int mark_page) {
	pxe_t			*pgdir;
	pxe_t			*ptep;
	uint64_t		pde;
	uint64_t		pte;
	unsigned		pg_offset;
	unsigned		pde_mask = (1 << pd_bits) - 1;
	paddr_t			paddr;
	uintptr_t		vaddr = (uintptr_t)addr;


#ifndef NDEBUG
	if(prp == NULL) crash();
#endif
	if(prp->memory == NULL)  {
		pgdir = pgtbl_list->pgdir;
	} else {
		pgdir = prp->memory->cpu.pgdir;
#ifndef NDEBUG
		if(prp->memory->cpu.ptroot_paddr != rdpgdir() && (vaddr < CPU_USER_VADDR_END)) crash();
#endif
	}

	pde = PXE_GET(GENERIC_VTOPDIRP(pgdir, vaddr));

	if(!(pde & X86_PTE_PRESENT)) goto fail;

	if(pde & X86_PDE_PS) {
		pg_offset = vaddr & pde_mask;
		if(!(pde & (X86_PDE_USER1|X86_PDE_PRESENT))) goto fail;
		paddr = (pde & ~(X86_PTE_NX | pde_mask)) + pg_offset;

		*offset = PADDR_TO_SYNC_OFF(paddr);
		return PADDR_TO_SYNC_OBJ(paddr);
	}

	pg_offset = ADDR_OFFSET(vaddr);

	// We can use the currently active page table 
	// to check the PTE - much faster
	ptep = VTOPTEP(vaddr);
	pte = PXE_GET(ptep);
	pte = PXE_GET(ptep);
	if(!(pte & (X86_PTE_USER1|X86_PTE_PRESENT))) goto fail;
	paddr = (pte & PTE_PADDR_BITS) | pg_offset;

	if(mark_page) {
		struct pa_quantum	*pq;

		// This line is for the 386, which doesn't produce write faults
		// when we want it to.
		if(!(pte & X86_PTE_WRITE)) return (OBJECT *)-1;

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
