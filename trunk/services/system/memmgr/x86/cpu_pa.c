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
#include "mm_internal.h"

/*
 * FIX ME - the VADDR value (2^32 - 16MB) is chosen to handle both PAE and non
 * PAE even though all that is required for non PAE is 2^32 - 8MB.
 * This is the easiest and safest fix for now however we should make the value
 * depend on 'pae_enabledi' and a PR has been created for that (PR61562)
*/
#define TOP_VADDR	0xFF000000U

uintptr_t
cpu_sysvaddr_find(uintptr_t start, unsigned size) {
	uintptr_t	rover;
	uintptr_t	next;
	unsigned	pg_size;
	unsigned	found_size;
	unsigned	found_va;
	int			is_free;
	unsigned	flags;
	pxe_t		*pdep;
	pxe_t		*ptep;
	int			wrapped;

	found_va = VA_INVALID;
	wrapped = 0;
	found_size = 0;
	if((start == 0) || (start >= TOP_VADDR)) {
		start = PROCMEM_BASE;
	}
	rover = start;
	for( ;; ) {
		is_free = 1;
		pg_size = 1 << pd_bits; // assuming big page or free
		pdep = VTOPDIRP(rover);
		flags = PXE_GET_FLAGS(pdep);
		if(flags & X86_PDE_PRESENT) {
			if(flags & X86_PDE_PS) {
				is_free = 0;
				found_size = 0;
			} else {
				ptep = VTOPTEP(rover);
				if(PXE_GET_FLAGS(ptep) & (X86_PTE_PRESENT| X86_PTE_USER1)) {
					found_size = 0;
					is_free = 0;
				}
				pg_size = __PAGESIZE;
			}
		}
		next = (rover + pg_size) & ~(pg_size - 1);
		if(found_size == 0) found_va = rover;
		if(is_free) found_size += next - rover;
		if(found_size >= size) {
			return found_va;
		}
		if(next >= TOP_VADDR) {
			wrapped = 1;
			next = PROCMEM_BASE;
			found_size = 0;
		}
		rover = next;
		if(wrapped && (rover >= start)) return VA_INVALID;
	}
}

void *
cpu_early_paddr_to_vaddr(paddr_t p, unsigned size, paddr_t *l2mem) {
	uintptr_t	start_va;
	uintptr_t	va;
	paddr_t		l2;
	pxe_t		*pdep;
	pxe_t		*ptep;
	pxe_t		*kmap_pde;
	pxe_t		*kmap_pte;
	unsigned 	cr0;

	if(CPU_1TO1_IS_PADDR(p)) {
		return (void *)((uintptr_t)p + CPU_1TO1_VADDR_BIAS);
	}

/*
	section pointed to by l2mem (largest section in asinfo, hope it's enough ;)
	+-------------------------------------------------------------+
	| allocated paddr memory                      page table/dir  |
	| ---------------------------->             <-----------------|
    | |xxxxxxx|xx|xxxx|x|xxxxxxx|	              |xx|xx|xx|xx|xx||
    +-------------------------------------------------------------+
*/

	start_va = cpu_sysvaddr_find(va_rover, size);
	if(start_va == VA_INVALID) {
		crash();
	}
	va_rover = start_va + size;
	va = start_va;

	/* Turn off the write protection bit (ignores the R/O bit in the pxe */
	cr0 = rdcr0();
	ldcr0(cr0 & ~X86_MSW_WP_BIT);

	do {
		pdep = VTOPDIRP(va);
		if(!(PXE_GET_FLAGS(pdep) & X86_PDE_PRESENT)) {
			/* grab some mem */
			*l2mem -= __PAGESIZE;
			l2 = *l2mem + 1; // +1 since l2mem points to _last_ free paddr

			/* zero the pagetable before we stick it in the pagedir (smp) */	
			kmap_pde = VTOPDIRP(L2MAP_BASE);
			kmap_pte = VTOPTP(L2MAP_BASE);
			PXE_SET(kmap_pde, l2 | (X86_PTE_WRITE|X86_PTE_PRESENT));
			CPU_ZERO_PAGE(kmap_pte, __PAGESIZE, 0);
			PXE_SET(kmap_pde, 0); /* loose the mapping */
			PXE_SET(pdep, l2 | (X86_PTE_WRITE|X86_PTE_PRESENT));
			// Can't use flushtlb() here because it unconditionally
			// renables interrupts, and they're supposed to be left off
			// throughout this whole section.
			ldpgdir(rdpgdir()); /* make sure TLB is consistent with change */
		} 
		ptep = VTOPTEP(va);
		PXE_SET(ptep, ADDR_PAGE(p) | (X86_PTE_PRESENT|X86_PTE_WRITE));
		p += __PAGESIZE;
		va += __PAGESIZE;
		size -= __PAGESIZE;
	} while(size != 0);

	ldcr0(cr0);
	return (void *)start_va;
}

unsigned
cpu_whitewash(struct pa_quantum *pq) {
	//NYI: have to zero the quantum
	return 0;
}

__SRCVERSION("cpu_pa.c $Rev: 201702 $");
