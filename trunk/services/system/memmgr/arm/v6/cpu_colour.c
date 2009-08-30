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

static unsigned cache_linelen;

/*
 * Clean and invalidate dcache by virtual address
 */
static void
colour_flush_cache(unsigned vaddr)
{
	unsigned	end = vaddr + __PAGESIZE;

	do {
		__asm__ __volatile__("mcr	p15, 0, %0, c7, c14, 1" : : "r"(vaddr));
		vaddr += cache_linelen;
	} while (vaddr < end);
}

/*
 * Invalidate dcache by virtual address
 */
static void
colour_purge_cache(unsigned vaddr)
{
	unsigned	end = vaddr + __PAGESIZE;

	do {
		__asm__ __volatile__("mcr	p15, 0, %0, c7, c6, 1" : : "r"(vaddr));
		vaddr += cache_linelen;
	} while (vaddr < end);
}

struct kerargs_colour_operation {
	void		(*func)(unsigned);
	unsigned	colour;
	paddr_t		paddr;
};
static void	colour_operation(void (*func)(unsigned), unsigned, paddr_t);

static void
kerext_colour_operation(void *data)
{
	struct kerargs_colour_operation	*kap = data;

	colour_operation(kap->func, kap->colour, kap->paddr);
}

/*
 * Map paddr at a virtual address with required colour for cache operation
 */
static void
colour_operation(void (*func)(unsigned), unsigned colour, paddr_t paddr)
{
	unsigned	vaddr;
	pte_t		*pte;

	if (!KerextAmInKernel()) {
		struct kerargs_colour_operation	data;

		data.func = func;
		data.colour = colour;
		data.paddr = paddr;
		__Ring0(kerext_colour_operation, &data);
		return;
	}

	KerextLock();
	vaddr = ARM_V6_COLOUR_BASE + (colour << ADDR_OFFSET_BITS);
	pte = KTOPTEP(vaddr);

	*pte = paddr | arm_cpu->kpte_ro;
	arm_v4_dtlb_addr(vaddr);			// global TLB entry - no asid required
	arm_v6_dsb();

	func(vaddr);

	*pte = 0;
	arm_v4_dtlb_addr(vaddr);			// global TLB entry - no asid required
}

/*
 * Calculate number of cache colours for physical addresses
 */
unsigned
v6_cache_colours(void)
{
	unsigned	ctype;

	/*
	 * Calculate set size/associativity for dcache (bits 23:12)
	 * The ctype field contains the following info:
	 *
	 * | 11| 10| 9 8 7 6 | 5 4 3 | 2 | 1 0
	 * | P | 0 |    Size | assoc | M | Len
	 *
	 * P = 0 means no virtual alias restrictions exist
	 * P = 1 means virtual aliases can exist in cache
	 *
	 * M and Size encodes size of cache:       (M + 2) << (Size + 8)
	 * M and assoc encodes cache associtivity: (M + 2) << (Assoc - 1)
	 * len is encodes cache line length:       1 << (Len + 3)
	 *
	 * So, if P is non-zero, each cache set is cache_size/associativity
	 * If set_size > __PAGESIZE, we have set_size/__PAGESIZE colours.
	 */
	ctype = arm_mmu_cache_type() >> 12;
	cache_linelen = 1 << ((ctype & 0x3) + 3);
	if (ctype & (1 << 11)) {
		unsigned	S = (ctype >> 6) & 0xf;
		unsigned	A = (ctype >> 3) & 0x7;
		unsigned	set_size = 1 << ((S - A) + 9);

		return (set_size > __PAGESIZE) ? set_size >> 12 : 1;
	}
	return 1;
}

void 
cpu_colour_clean(struct pa_quantum *qp, int cacheop)
{
	paddr_t		paddr  = pa_quantum_to_paddr(qp);
	unsigned	colour = PAQ_GET_COLOUR(qp);
	void		(*func)(unsigned) = 0;

	switch (cacheop) {
	case COLOUR_CLEAN_PURGE:
		func = colour_purge_cache;
		break;

	case COLOUR_CLEAN_FLUSH:
		func = colour_flush_cache;
		break;

	case COLOUR_CLEAN_ZERO_FLUSH:	// FIXME: not yet implemented
	default:
		crash();
	}

	if (colour == COLOUR_PA(paddr) && CPU_1TO1_IS_PADDR(paddr)) {
		/*
		 * Can use the 1-1 mapping to manipulate the cache
		 */
		func(paddr + CPU_1TO1_VADDR_BIAS);
	}
	else {
		/*
		 * Need to create a mapping with required colour
		 */
		colour_operation(func, colour, paddr);
	}
}

__SRCVERSION("cpu_colour.c $Rev: 170760 $");
