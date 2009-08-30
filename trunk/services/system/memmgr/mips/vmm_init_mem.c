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


void 
vmm_init_mem(int phase) {
	unsigned    			    i;
	unsigned					num_colours;
	uintptr_t					free_base;
	unsigned					colour_size;
	struct cpuinfo_entry        *cpu;
	struct cacheattr_entry      *cache;
	struct asid_mapping			*asid_map;
	unsigned					size;
	unsigned					dcache_ways;
	uintptr_t					max_pgsz;

	if(phase != 0) {
		pa_start_background();
		fault_init();
		lock_init();
		return;
	}

	cpu = SYSPAGE_ENTRY(cpuinfo);

	cache = &SYSPAGE_ENTRY(cacheattr)[cpu->ins_cache];
	icache_lines = cache->num_lines;
	icache_lsize = cache->line_size;
	icache_size = icache_lines * icache_lsize;
	icache_lines_per_page = __PAGESIZE/icache_lsize;
	icache_flags = cache->flags;

	cache = &SYSPAGE_ENTRY(cacheattr)[cpu->data_cache];
	if(cache->next != CACHE_LIST_END) {
		mm_flags |= MM_FLAG_MULTIPLE_DCACHE_LEVELS;
	}
	dcache_ways = cache->ways;
	dcache_lines = cache->num_lines;
	dcache_lsize = cache->line_size;
	dcache_size = dcache_lines * dcache_lsize;
	dcache_lines_per_page = __PAGESIZE/dcache_lsize;

#ifdef PTEHACK_SUPPORT
	ptehack_ptr = SYSPAGE_CPU_ENTRY(mips, ptehack);
	ptehack_num = _syspage_ptr->un.mips.ptehack.entry_size / sizeof(*ptehack_ptr);
#endif

	/*
	 * The NEC 41xx series has 1K page support. Implemented poorly. For
	 * example, we have to put 0x1800 in the pagemask register to get a
	 * 4K page, rather than 0x0000 on other R4000's. We also have to adjust
	 * the location/size of the PFN field in the entrylo[0,1] registers.
	 */
	pfn_topshift = cpu->flags & MIPS_CPU_FLAG_PFNTOPSHIFT_MASK;

	/* Figure out how many tlb entries we have */
	if(MIPS_PRID_COMPANY(getcp0_prid()) != 0) {
		unsigned        cfg1 = CP0REG_SEL_GET(16,1);

		num_tlbs = 1 +
				((cfg1 & MIPS_CONFIG1_MMUSIZE_MASK) >> MIPS_CONFIG1_MMUSIZE_SHIFT);
	} else {
		num_tlbs = discover_num_tlb(TLB_IDX_SHIFT);
	}

	if(!(__cpu_flags & MIPS_CPU_FLAG_NO_WIRED)) {
		setcp0_wired(0);
	}

#if defined(VARIANT_r3k)
	num_colours = 1;
#else 
	// Remember what the bits are on in the page mask register for a 4K page.
	pgmask_4k = getcp0_pagemask();

	/*
	 * What cache coherency number should we use for cacheable pages?
	 */
	pt_cacheable_attr = (getcp0_config() & MIPS_CONFIG_KSEG0_COHERENCY) << MIPS_TLB_LO_CSHIFT;

	if((dcache_ways != 0) && !(cache->flags & CACHE_FLAG_VIRT_IDX)) {
		// This is a new startup, so we  can depend on the CACHE_FLAG_VIRT_IDX 
		// flag to tell us if we have colours or not.
		num_colours = 1;
	} else if(_syspage_ptr->num_cpu > 1) {
		// Old startup, but SMP machines need a physically indexed dcache 
		// in order to maintain cache coherency
		num_colours = 1;
	} else {
		if(dcache_ways == 0) dcache_ways = 1; // old startup
		num_colours = dcache_size / (__PAGESIZE * dcache_ways);
	}
#endif

	pa_init(num_colours);

	free_base = perm_map_init();

	colour_size = num_colours * __PAGESIZE;
	free_base = colour_init(free_base, colour_size);

	/* initialize the xfer_tbl */
	max_pgsz = 1 << ((cpu->flags & MIPS_CPU_FLAG_MAX_PGSIZE_MASK) >> MIPS_CPU_FLAG_MAX_PGSIZE_SHIFT);
	max_pgsz = xfer_init(free_base, max_pgsz);

	pgszlist_init(max_pgsz);

	wire_init();

	size = sizeof(*asid_map) * NUM_PROCESSORS;
	asid_map = _scalloc(size);
	if(asid_map == NULL) {
		crash();
	}
	for(i = 0; i < NUM_PROCESSORS; ++i) {
		asid_map->rotor = 1;
		gbl_asid_map[i] = asid_map;
		++asid_map;
	}

	clean_init();
}

__SRCVERSION("vmm_init_mem.c $Rev: 157044 $");
