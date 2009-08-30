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


// Invalidate data cache using physical address
// only could be use when addr and size all cache line aligned.
// assuming full data cache area is used as cache
void 
mem_page_cache_invalidate_phy(paddr_t phyaddr, uint32_t size) {
	uintptr_t ptr,end;
	uint32_t tag,index, dcache_size, linesize;
	struct cacheattr_entry *cacheattr;

	cacheattr = SYSPAGE_ENTRY(cacheattr);
	cacheattr += SYSPAGE_ENTRY(cpuinfo)->data_cache;
	dcache_size = cacheattr->num_lines * cacheattr->line_size;
	linesize = cacheattr->line_size;

	end = (phyaddr + size + linesize - 1) & ~(linesize - 1);
	phyaddr &= ~(linesize - 1);
	// We use the sh_tlb_in32/sh_tlb_out32 routines here because they meet the same
	// requirements we need to meet for accessing the cache tables through RAM -- on
	// SH4 architectures the code runs through the P2 (non-cached) area, while on
	// SH4a architectures the code runs normally.
	for(index=0; index<dcache_size; index += linesize) {
		ptr = SH4_DCACHE_ADDRESS_ARRAY	+ index;
		tag = REAL_ADDR(sh_tlb_in32(ptr)) | (index & 0x3e0);
		if( (tag >= phyaddr) && (tag<end) ) {
			// invalidate the cache
			sh_tlb_out32(ptr,0);
		}
	}
}


void 
cpu_colour_clean(struct pa_quantum *qp, int cacheop) {
	paddr_t				paddr;
	uintptr_t 			ptr,end;
	unsigned			colour;

	paddr = pa_quantum_to_paddr(qp);

	switch(cacheop) {
	case COLOUR_CLEAN_ZERO_FLUSH:
		colour = PAQ_GET_COLOUR(qp);
		if(colour != PAQ_COLOUR_NONE && colour != COLOUR_PA(paddr)) {
			mem_page_cache_invalidate_phy(paddr, __PAGESIZE);
			PAQ_SET_COLOUR(qp, PAQ_COLOUR_NONE);
		}
		if ( SH_PHYS_IN_1TO1(paddr) ) {
		    CPU_ZERO_PAGE(SH_PHYS_TO_P1(paddr), __PAGESIZE, 0);
		} else {
		    //RUSH:need to fix for 32bit physical memory (>0.5G)
		    // We can get away with this for now, since COLOUR_CLEAN_ZERO_FLUSH
		    // is not currently used (it's only referenced in cpu_whitewash()
		    // which in turn isn't used).
		    crash();
		}
		// Fall into flush code
	case COLOUR_CLEAN_FLUSH:
		colour = PAQ_GET_COLOUR(qp);
		// If the colour has changed or we're outside the 1-to-1 area, use the flush
		// method that is slower but handles these cases.
		if( (colour != PAQ_COLOUR_NONE && colour != COLOUR_PA(paddr)) || ( !SH_PHYS_IN_1TO1(paddr) ) ) {
			mem_page_cache_invalidate_phy(paddr, __PAGESIZE);
		} else {
			for(ptr=SH_PHYS_TO_P1(paddr),end=ptr+__PAGESIZE;ptr<end;ptr+=SH4_DCACHE_LINESIZE) {
				dcache_flush(ptr);
			}
		}
		PAQ_SET_COLOUR(qp, PAQ_COLOUR_NONE);
		break;
	case COLOUR_CLEAN_PURGE:
		mem_page_cache_invalidate_phy(paddr, __PAGESIZE);
		break;
	default: break;
	}
}

__SRCVERSION("mem_cache.c $Rev: 159814 $");
