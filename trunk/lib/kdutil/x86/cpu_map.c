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

#include <sys/mman.h>
#include <x86/cpu.h>
#include <x86/priv.h>
#include <stdlib.h>
#include "kdintl.h"

typedef union pxe {
	uint32_t		pxe32;
	uint64_t		pxe64;
} pxe_t; // used for both page directory entries and page table entries.

//Identify 1-to-1 mapping vaddrs & paddrs
#define CPU_SYSTEM_PADDR_START	0
#define CPU_SYSTEM_PADDR_END	((256*1024*1024)-1)
#define CPU_1TO1_VADDR_BIAS		0xe0000000u
#define CPU_1TO1_IS_VADDR(v)	((__cpu_flags & X86_CPU_PSE) && ((v) >= CPU_1TO1_VADDR_BIAS) && ((v) <= (CPU_1TO1_VADDR_BIAS+CPU_SYSTEM_PADDR_END)))
#define CPU_1TO1_IS_PADDR(p)	((__cpu_flags & X86_CPU_PSE) && ((p) <= CPU_SYSTEM_PADDR_END))

static int	pae_enabled;

/* This is used when PAE is disabled */
/* To make these work the last entry of the page directory points to itself */

#define PG1_REMAP        0x3ff
#define PG1_PDIRADDR     (PG1_REMAP << 22 | PG1_REMAP << 12)	// Pointer to start of page directory
#define PG1_PTEADDR      (PG1_REMAP << 22)						// Pointer to first page table

#define V1TOPDIRP(v)    (uint32_t *)(PG1_PDIRADDR | (((uint32_t)(v))>>20&~3))		// Pointer to page directory entry
#define V1TOPTEP(v)     (uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&~3))		// Pointer to page table entry
#define V1TOPTP(v)      (uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&0x3ff000))	// Pointer to start of page table
#define V1TOPADDR(v)	((*V1TOPTEP(v)&~(PAGESIZE-1))|((uint32_t)(v)&(PAGESIZE-1)))	// Physical address


/* This is used when PAE is enabled */
/* To make these work the last 4 entries of the PDPT point to 4 pages within a 16k page directory */

#define PG2_PDIRADDR    0xffffc000		// Pointer to start of page directory
#define PG2_PTEADDR     0xff800000		// Pointer to first page table

#define V2TOPDIRP(v)    (uint64_t *)(PG2_PDIRADDR | (((uint32_t)(v))>>18&~7))		// Pointer to page directory entry
#define V2TOPTEP(v)     (uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&~7))		// Pointer to page table entry
#define V2TOPTP(v)      (uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&0x7ff000))	// Pointer to start of page table
#define V2TOPADDR(v)    ((*V2TOPTEP(v)&~(__PAGESIZE-1))|((uint32_t)(v)&(__PAGESIZE-1)))	// Physical address

#define				VA_START_SEARCH	0xe0000000
uint64_t			save_pte1;
uint64_t			save_pte2;
uintptr_t			va_rover = VA_START_SEARCH;

#define MAP_REG1	0x80000000
#define V1MAP_REG2	(MAP_REG1+(1 << 22))
#define V2MAP_REG2	(MAP_REG1+(1 << 21))


void
cpu_init_map(void) {
	if(SYSPAGE_ENTRY(cpuinfo)[0].flags & X86_CPU_PAE) {
		if(rdcr4() & X86_CR4_PAE) {
			pae_enabled = 1;
		}
	}
}


static uintptr_t
find_va(unsigned size) {
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

	found_va = 0;
	wrapped = 0;
	found_size = 0;
	rover = va_rover;
	for( ;; ) {
		is_free = 1;
		if(pae_enabled) {
			pdep = (pxe_t *)V2TOPDIRP(rover);
			pg_size = 1 << 21;
		} else {
			pdep = (pxe_t *)V1TOPDIRP(rover);
			pg_size = 1 << 22;
		}
		flags = pdep->pxe32;
		if(flags & X86_PDE_PRESENT) {
			if(flags & X86_PDE_PS) {
				is_free = 0;
				found_size = 0;
			} else {
				if(pae_enabled) {
					ptep = (pxe_t *)V2TOPTEP(rover);
				} else {
					ptep = (pxe_t *)V1TOPTEP(rover);
				}
				if(ptep->pxe32 & (X86_PTE_PRESENT| X86_PTE_USER1)) {
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
			va_rover = found_va + size;
			return found_va;
		}
		if(next >= PG2_PTEADDR) {
			wrapped = 1;
			next = VA_START_SEARCH;
			found_size = 0;
		}
		rover = next;
		if(wrapped && (rover >= va_rover)) return 0;
	}
}


void *
cpu_map(paddr64_t paddr, unsigned size, unsigned prot, int colour, unsigned *mapped_len) {
	unsigned	off;
	uintptr_t	vaddr;
	unsigned	cr0;

	cr0 = rdcr0();
	if(!(cr0 & X86_MSW_PG_BIT)) {
		// paging is _off_
		*mapped_len = size;
		return (void *)(uintptr_t)paddr;
	}
	if(CPU_1TO1_IS_PADDR(paddr) && !(prot & PROT_NOCACHE)) {
		// We're assuming that a mapping request never crosses from inside
		// the 1-to-1 area to outside of it.
		if(mapped_len != NULL) *mapped_len = size;
		return (void *)((uintptr_t)paddr + CPU_1TO1_VADDR_BIAS);
	}

	off = paddr & (__PAGESIZE - 1);
	paddr -= off;

	paddr |= X86_PTE_PRESENT;
	if(prot & PROT_WRITE) paddr |= X86_PTE_WRITE;
	if(prot & PROT_NOCACHE) paddr |= X86_PTE_CD;

	if(private->kdebug_info == NULL) {
		uintptr_t	va;
		uintptr_t	end;
		unsigned	vaddr;
		paddr64_t	l2;
		pxe_t		*ptep;
		pxe_t		*pdep;

		// Haven't started the kernel yet - this is a permanent allocation
		va = find_va(size);
		if(va == 0) return NULL;

		vaddr = va;
		end = va + size - 1;
		ldcr0(cr0 & ~X86_MSW_WP_BIT);
		do {
			if(pae_enabled) {
				ptep = (pxe_t *)V2TOPDIRP(va);
			} else {
				ptep = (pxe_t *)V1TOPDIRP(va);
			}

			if(!(ptep->pxe32 & X86_PDE_PRESENT)) {
				l2 = alloc_pmem(__PAGESIZE, __PAGESIZE);
				if(l2 == ~(paddr64_t)0) {
					if(vaddr > va) {
						// Unmap what we've done
						cpu_unmap((void *)vaddr, va - vaddr);
					}
					ldcr0(cr0);
					return NULL;
				}

				if(pae_enabled) {
					ptep->pxe64 = l2 | X86_PTE_WRITE|X86_PTE_PRESENT;
					pdep = (pxe_t *)V2TOPTP(va);
				} else {
					ptep->pxe32 = l2 | X86_PTE_WRITE|X86_PTE_PRESENT;
					pdep = (pxe_t *)V1TOPTP(va);
				}
				memset(pdep, 0, __PAGESIZE); // zero the L2
			}
			if(pae_enabled) {
				ptep = (pxe_t *)V2TOPTEP(va);
				ptep->pxe64 = paddr;
			} else {
				ptep = (pxe_t *)V1TOPTEP(va);
				ptep->pxe32 = paddr;
			}
			paddr += __PAGESIZE;
			va += __PAGESIZE;
		} while(va < end);
		ldcr0(cr0);
		ldpgdir(rdpgdir());
		vaddr += off;
		if(mapped_len != NULL) *mapped_len = size;
		return (void *)vaddr;
	}

	// Kernel is up - this is a temporary mapping. Only have to allow
	// one at a time.
	ldcr0(cr0 & ~X86_MSW_WP_BIT);
	if(pae_enabled) {
		save_pte1 = *V2TOPDIRP(MAP_REG1);
		save_pte2 = *V2TOPDIRP(V2MAP_REG2);
		*V2TOPDIRP(MAP_REG1) = paddr;
		*V2TOPDIRP(V2MAP_REG2) = paddr + __PAGESIZE;
		ldpgdir(rdpgdir());
		vaddr = (uintptr_t)V2TOPTP(MAP_REG1);
	} else {
		save_pte1 = *V1TOPDIRP(MAP_REG1);
		save_pte2 = *V1TOPDIRP(V1MAP_REG2);
		*V1TOPDIRP(MAP_REG1) = paddr;
		*V1TOPDIRP(V1MAP_REG2) = paddr + __PAGESIZE;
		ldpgdir(rdpgdir());
		vaddr = (uintptr_t)V1TOPTP(MAP_REG1);
	}
	// Leave the MSW WP bit off so we can store into the temp mapping.
	// The cpu_unmap() code will turn it back on.
	if(mapped_len != NULL) *mapped_len = min(size, (__PAGESIZE*2) - off);
	return (void *)(vaddr + off);
}


void
cpu_unmap(void *p, unsigned size) {
	unsigned	cr0;

	cr0 = rdcr0();
	if(!CPU_1TO1_IS_VADDR((uintptr_t)p) && (cr0 & X86_MSW_PG_BIT)) {
		ldcr0(cr0 & ~X86_MSW_WP_BIT);
		if(private->kdebug_info == NULL) {
			uintptr_t	va;
			uintptr_t	end;
			pxe_t		*ptep;

			va = (uintptr_t)p & ~(__PAGESIZE-1);
			va_rover = va;
			end = (uintptr_t)p + size - 1;
			do {
				if(pae_enabled) {
					ptep = (pxe_t *)V2TOPTEP(va);
					ptep->pxe64 = 0;
				} else {
					ptep = (pxe_t *)V1TOPTEP(va);
					ptep->pxe32 = 0;
				}
				va += __PAGESIZE;
			} while(va < end);
		} else if(pae_enabled) {
			*V2TOPDIRP(MAP_REG1) = save_pte1;
			*V2TOPDIRP(V2MAP_REG2) = save_pte2;
		} else {
			*V1TOPDIRP(MAP_REG1) = save_pte1;
			*V1TOPDIRP(V1MAP_REG2) = save_pte2;
		}
		ldpgdir(rdpgdir());
		ldcr0(cr0 | X86_MSW_WP_BIT);
	}
}


unsigned
cpu_vaddrinfo(void *p, paddr64_t *paddrp, unsigned *lenp) {
	uintptr_t			vaddr = (uintptr_t)p;
	uint64_t			pte;
	unsigned			pagemask;
	unsigned			prot;

	if((rdcr0() & X86_MSW_PG_BIT) == 0) {
		// paging _not_ enabled
		*paddrp = vaddr;
		return PROT_READ|PROT_WRITE|PROT_EXEC;
	}

	if(pae_enabled) {
		pte = *V2TOPDIRP(vaddr);
		pagemask = (2 * 1024 * 1024) - 1;
	} else {
		pte = *V1TOPDIRP(vaddr);
		pagemask = (4 * 1024 * 1024) - 1;
	}

	if(!(pte & X86_PDE_PS) && (pte & X86_PTE_PRESENT)) {
		// 4K page
		pagemask = 0xfff;
		if(pae_enabled) {
			pte = *V2TOPTEP(vaddr);
		} else {
			pte = *V1TOPTEP(vaddr);
		}
	}
	if(!(pte & X86_PTE_PRESENT)) {
		return PROT_NONE;
	}
	*paddrp = (pte & ~(X86_PTE_NX | pagemask)) | (vaddr & pagemask);
	*lenp = (pagemask + 1) - (vaddr & pagemask);
	prot = MAP_PHYS; // to avoid a PROT_NONE return
	if(pte & X86_PTE_PRESENT) {
		prot |= PROT_READ;
		if(pte & X86_PTE_WRITE) prot |= PROT_WRITE;
		if(!pae_enabled || !(__cpu_flags & X86_CPU_NX) || !(pte & X86_PTE_NX)) {
			prot |= PROT_EXEC;
		}
	}
	return prot;
}
