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
#include <sh/cpu.h>
#include <sh/ccn.h>			// TLB definitions
#include <kernel/macros.h>  // pick up KILO and MEG macros
#include "kdintl.h"



/*
 * sh_in32 / sh_out32 routines
 * 
 * These routines are sufficient to meet the requirements of updating
 * tlbs on both SH4 and SH4a platforms (which means they're a little slower
 * than they need to be on SH4a, but they're not used too much so that's ok).
 * 
 * sh_in32 and sh_out32 are function pointers.  They are assigned P2 addresses of
 * the sh4_tlb_* routines.
 * 
 */

unsigned (*sh_in32)(unsigned); 
void (*sh_out32)(unsigned,unsigned);

unsigned sh4_tlb_in32(unsigned p) {
	unsigned v;
	__asm__ __volatile__(
		"	mov.l	@%1,%0;"
		"	nop;nop;nop;nop;"  // h/w manual says we can't branch out of the p2 area for 8
		"	nop;nop;nop;nop;"  // instructions after the mov (7751 h/w manual, section 3.7)
		:"=&r" (v)
		: "r" (p)
		: "pr", "memory"
	);
	return v;
}
void sh4_tlb_out32(unsigned p, unsigned v) {
	__asm__ __volatile__(
		"	mov.l	%1,@%0;"
		"	nop;nop;nop;nop;"   // h/w manual says we can't branch out of the p2 area for 8
		"	nop;nop;nop;nop;"   // instructions after the mov (7751 h/w manual, section 3.7)
		: 
		: "r" (p), "r" (v)
		: "pr", "memory"
	);
}


/*
 * tlb_flush_va
 * 
 * flush all data tlbs that map anything between start and end.  Ignore ASIDs
 * just to make it simpler.
 */
static void
tlb_flush_va(uintptr_t start, uintptr_t end) {
	unsigned 	index;
	uint32_t	ptel, pteh;
	uint32_t	mask;
	uintptr_t	vaddr, ppteh;

	// flush utlb buffer
	for(index=0;index<SH4_UTLB_SIZE;index++) {
		ptel = sh_in32( (index<<8) + SH4_UTLB_DATA_ARRAY1 );
		mask = SH_PTE_PGSIZE_MASK(ptel);
		ppteh = (index<<8) + SH4_UTLB_ADDRESS_ARRAY;
		pteh = sh_in32( ppteh );
		vaddr = pteh & mask;
		if( !( (end < vaddr) || (start > vaddr+SH_PTE_PGSIZE(ptel)-1) ) ) {
			sh_out32(ppteh,0);
		}
	}
}

/*
 * write_tlb
 * 
 * Create a single-page tlb mapping the given vaddr/paddr with the given protection
 */
void
write_tlb (paddr64_t paddr, uintptr_t vaddr, unsigned prot) {
	unsigned ptel, pteh;

	// Create a ptel from the paddr with appropriate PROT bits
	ptel = (paddr & ~(__PAGESIZE-1)) | SH_CCN_PTEL_SZ0 | SH_CCN_PTEL_V | SH_CCN_PTEL_PR(2);
	if ( !(prot & PROT_NOCACHE) ) {
		ptel |= SH_CCN_PTEL_C;
	}
	if ( prot & PROT_WRITE ) {
		ptel |= SH_CCN_PTEL_PR(1);
	}
	//The pteh is created from the existing PTEH.asid and the vaddr page address
	pteh = sh_in32(SH_MMR_CCN_PTEH);
	pteh = (vaddr & ~(__PAGESIZE-1)) | (pteh & SH_CCN_PTEH_ASID_M);

	sh_out32(SH_MMR_CCN_PTEH, pteh);
	sh_out32(SH_MMR_CCN_PTEL, ptel);
	__asm__ __volatile__( "ldtlb" );

}


// We'll place our temporary mappings in the P3 area, which is mostly
// unused otherwise.
#define TEMP_MAP_BASE		SH_P3BASE
#define TEMP_MAP_MAX_END	TEMP_MAP_BASE+MEG(4)


void
cpu_init_map(void) {
	sh_in32 = (void*)SH_PHYS_TO_P2(SH_P1_TO_PHYS(sh4_tlb_in32));
	sh_out32 = (void*)SH_PHYS_TO_P2(SH_P1_TO_PHYS(sh4_tlb_out32));
}


void *
cpu_map(paddr64_t paddr, unsigned size, unsigned prot, int colour, unsigned *mapped_len) {
	uintptr_t 	p;
	unsigned	offset;
	unsigned	mapping_size;
	
	// Give a 1-1 address if we can.
	if((paddr < SH_P1SIZE) && (colour == CPU_MAP_COLOUR_NONE)) {
		if (mapped_len != NULL) {
			*mapped_len = SH_P1SIZE - (uintptr_t)paddr;
		}
		if (prot & PROT_NOCACHE) {
			p = SH_PHYS_TO_P2((uintptr_t)paddr);
		} else {
			p = SH_PHYS_TO_P1((uintptr_t)paddr);
		}
		return (void*)p;
	}
	
	// Deal with paddrs outside the 1-1 area or with a colour that doesn't match
	// the 1-1 area.  We need to create a new map.
	
	// ZZZ only supporting small pages for now

	// First, come up with a vaddr that matches the given colour.  Since these
	// are very temporary, we don't need to be too fussy about where we set up
	// the mapping.  We also only need to support a single mapping at a time.
	if(colour < 0) colour = 0;
	p = TEMP_MAP_BASE + (colour * __PAGESIZE);

	offset = paddr & (__PAGESIZE-1);
	paddr -= offset;
	mapping_size = __PAGESIZE - offset;
	
	// Make sure there isn't a TLB hanging around that also covers this area
	tlb_flush_va(p, p+mapping_size);

	// Add the new tlb
	write_tlb(paddr, p, prot);
	
	if(size < mapping_size) mapping_size = size;
	if(mapped_len != NULL) *mapped_len = mapping_size;
	return (void *)(p + offset);
}


void
cpu_unmap(void *p, unsigned size) {
	// clear out the TLB
	if ( SH_IS_P3((unsigned)p) && (unsigned)p<TEMP_MAP_MAX_END) {
		tlb_flush_va((unsigned)p, (unsigned)p+size);
	}
}


unsigned
cpu_vaddrinfo(void *p, paddr64_t *paddrp, unsigned *lenp) {
	uintptr_t	vaddr = (uintptr_t)p;

	if(SH_IS_P1(vaddr) || SH_IS_P2(vaddr) || SH_IS_P4(vaddr)) {
		*paddrp = vaddr & ~0xc0000000;
		*lenp = SH_P0SIZE - (unsigned)*paddrp;
		return PROT_READ|PROT_WRITE|PROT_EXEC;
	}
	return PROT_NONE;
}
