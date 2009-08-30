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


static int      dcache_hit_workaround;

#define PAGE_NOT_PRESENT	0
#define PAGE_INVALID		1
#define PAGE_PRESENT		2

/*
 * See if a page is in the cache. Read the tags and
 * if valid, compare the physical address.
 *
 * Returns: 
 *      PAGE_NOT_PRESENT if page wasn't in cache, even in tags
 *      PAGE_INVALID if page was in some tags but lines were invalid
 *      PAGE_PRESENT if page in tags and line(s) valid
 */
static int 
page_in_dcache(paddr_t paddr) {
	register uint32_t   cacheaddr;
	register uint32_t   tag;
	register int        i;
	int                 lines;
	register int        incache = PAGE_NOT_PRESENT;
	register uint32_t   save_taglo;
	register uint32_t   save_ecc;
	unsigned            prev;

	/* Don't use InterruptEnable/InterruptDisable in this routine
		 - we haven't set up *__shadow_imask yet
	*/
	prev = getcp0_sreg();
	setcp0_sreg(prev & ~MIPS_SREG_IE);

	save_taglo = getcp0_taglo();

	/* save ecc register */
	save_ecc = CP0REG_GET(26);

	cacheaddr = MIPS_R4K_K0BASE;

	/* tags store upper 24 bits of physical address */
	paddr = paddr >> ADDR_OFFSET_BITS;
	lines = dcache_lines;

	for (i = 0; i < lines; i++) {
		asm volatile (
				".set mips3             ;"
				".set noreorder         ;"
				"cache  5,0(%0);        ;" /* idx load tag dcache */
				"nop                    ;"
				"nop                    ;"
				".set reorder           ;"
				".set mips2             ;"
				: /* no outputs */
				: "r" (cacheaddr)); 

		/* tag value stored in CP0_TAGLO */
		tag = getcp0_taglo();

		/* check ptaglo */
		if (((tag & MIPS_TAGLO_PTAGLO) >> MIPS_TAGLO_PTAGLO_SHIFT) == paddr) {
			/*
			 * This line belongs to the page. Now
			 * get the cache state- if it is non-zero
			 * the line is valid and we are done.
			 */
			if (tag & MIPS_TAGLO_PSTATE) {
				incache = PAGE_PRESENT;
				goto done;
			} else {
				incache = PAGE_INVALID;
			}
		}
		cacheaddr += dcache_lsize;
	}

done:
	setcp0_taglo(save_taglo);

	/* restore ecc register */
	CP0REG_SET(26, save_ecc);

	setcp0_sreg(prev);
	return incache;
}


/*
 * clean_cache_page:
 *      Redo per-page cache scrubbing. Since the chip bugs relating
 *      to HIT_* operations in the dcache can't be worked around 
 *      reliably, take a different approach. March through the specified
 *      cache and read the tags. If the tag matches the paddr passed in,
 *      then do an indexed invalidate or indexed writeback-invalidate
 *      on the line. Since indexed operations are being performed, kseg0
 *      addresses can be used, so there's no need to use ProcNto's
 *      page tables and go through the tlb.
 */

static void
zero_page(void *va) {
	memset(va, 0, __PAGESIZE);
}


void 
cpu_colour_clean(struct pa_quantum *qp, int cacheop) {
	void        		(*cache_rtn)(void *);
	unsigned			colour;
	paddr_t				paddr;

	paddr = pa_quantum_to_paddr(qp);

	/*
	 * Flush/purge routine will use HIT_* cache ops. Make
	 * sure we inspect the correct part of the cache.
	 */
	colour = PAQ_GET_COLOUR(qp);
	if(colour == PAQ_COLOUR_NONE) {
		colour = COLOUR_VA((uintptr_t)paddr);
	}

	cache_rtn = NULL;
	switch(cacheop) {
	case COLOUR_CLEAN_ZERO_FLUSH:
		colour_operation(zero_page, colour, paddr);
		// Fall into flush code
#if defined(VARIANT_r3k)
	// fall through
	case COLOUR_CLEAN_FLUSH:
	case COLOUR_CLEAN_PURGE:
		return;
#else
	// fall through
	case COLOUR_CLEAN_FLUSH:
		if(dcache_hit_workaround) {
			/* tag compares look at upper bits of the physical address */
			r4k_flush_dcache_page_hitwa(paddr >> ADDR_OFFSET_BITS);
			return;
		}
		cache_rtn = r4k_flush_dcache_page;
		break;
	case COLOUR_CLEAN_PURGE:
		if(dcache_hit_workaround) {
			/* tag compares look at upper bits of the physical address */
			r4k_purge_dcache_page_hitwa(paddr >> ADDR_OFFSET_BITS);
			return;
		}
		cache_rtn = r4k_purge_dcache_page;
		break;
#endif
	default:
		crash();
		return;
	}
	colour_operation(cache_rtn, colour, paddr);

#ifdef CACHE_DEBUG
	if(page_in_dcache(paddr) == PAGE_PRESENT) {
		kprintf("page 0x%x still in dcache!!\n", paddr);
		crash();
	}
#endif
}


/*
 * clean_init:
 *
 * See if HIT_* cacheops work in the dcache.
 * Allocate a page, write it, and then try to get to
 * out of the cache with the HIT_WRITEBACK_INVAL cacheops.
 * When done, call page_in_dcache to see if the page is still
 * resident in the cache or not.
 *
 * Sets global dcache_hit_workaround.
 *
 * NOTE: We should do this test in startup and set a bit in cpuinfo->flags
 *       field. Saves some memory.... bstecher
 */
void
clean_init(void) {
	char        		*ptr;
	int         		i;
	struct pa_quantum	*qp;
	paddr32_t			paddr;
	memsize_t			resv = 0;
	part_id_t		mpid = mempart_getid(NULL, sys_memclass_id);	// FIX ME - system partition ?
	switch(MIPS_PRID_COMPANY_IMPL(getcp0_prid())) {
	case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_NONE,MIPS_PRID_IMPL_4600): 
	case MIPS_PRID_MAKE_COMPANY_IMPL(MIPS_PRID_COMPANY_NONE,MIPS_PRID_IMPL_4700): 

		if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
			crash();
		}
		qp = pa_alloc(__PAGESIZE, __PAGESIZE, PAQ_COLOUR_NONE, 0, NULL, restrict_proc, resv);
		if (!qp) {
			MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
			crash();
		}
		MEMCLASS_PID_USE(NULL, mempart_get_classid(mpid), __PAGESIZE);
		paddr = pa_quantum_to_paddr(qp);
		ptr = (char *)CPU_P2V(paddr);
		
		for(i = 0 ; i < dcache_lines_per_page; i++, ptr += dcache_lsize) {
			*ptr = i;
		}
		ptr = (char *)CPU_P2V(paddr);
	
		if(page_in_dcache(paddr) != PAGE_PRESENT) {
			crash();
		}
	
		/* try to flush/invalidate the page with no work-around */
		r4k_flush_dcache_page(ptr);
	
		/* now see if the page is still in the cache */
		if(page_in_dcache(paddr) == PAGE_PRESENT) {
			/* need to do the workaround */
			dcache_hit_workaround = 1;
			r4k_flush_dcache_page_hitwa(paddr >> ADDR_OFFSET_BITS);
		}
		MEMCLASS_PID_FREE(NULL, mempart_get_classid(mpid), NQUANTUM_TO_LEN(qp->run));	// do before pa_free_list
		pa_free_list(qp, MEMPART_DECR(mpid, NQUANTUM_TO_LEN(qp->run)));
		break;
	default: break;	
	}
}

__SRCVERSION("mem_cache.c $Rev: 168445 $");
