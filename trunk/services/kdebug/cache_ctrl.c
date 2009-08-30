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

#include "kdebug.h"

static void
cache_one(struct cacheattr_entry *cache, uintptr_t base, size_t len, int flags) {
	uintptr_t				vaddr;
	size_t					cache_len;

	vaddr = base & ~(cache->line_size-1);
	cache_len = len + (base - vaddr);
	while(cache_len != 0) {
		unsigned	lines;
		paddr_t		addr;
		size_t		valid_len;

		valid_len = cache_len;
		addr = vaddr;
		if(cache->flags & CACHE_FLAG_CTRL_PHYS) {
			if(vaddrinfo(NULL, vaddr, &addr, &valid_len) == PROT_NONE) {
				unsigned page = SYSPAGE_ENTRY(system_private)->pagesize;

				/* make vaddr skip to the next page boundary */
				addr = (vaddr + page) & (~(page-1));
				if(cache_len < (addr - vaddr)) {
					cache_len = 0;
				} else {
					cache_len -= (addr - vaddr);
					vaddr = addr;
				}
				continue;
			}
		}
		lines = (valid_len + cache->line_size - 1) / cache->line_size;
		while(lines != 0) {
			unsigned	done_lines;
			size_t		done_size;

			//NYI: need to make handle 64 bit paddrs....
			done_lines = cache->control((paddr32_t)addr, lines, flags, cache, _syspage_ptr);
			if(done_lines == 0) return; /* whole cache handled */
			done_size = done_lines * cache->line_size;
			addr += done_size;
			lines -= done_lines;
		}
		cache_len -= valid_len;
	}
}

static void
cache_iterate(int cache_idx, uintptr_t base, size_t len, int flags, unsigned term_flag) {
	struct cacheattr_entry	*cache_base;
	struct cacheattr_entry	*cache;
	struct cacheattr_entry	*subset_cache;

	subset_cache = NULL;
	cache_base = SYSPAGE_ENTRY(cacheattr);
	for(;;) {
		if(cache_idx == CACHE_LIST_END) break;
		cache = &cache_base[cache_idx];
		if(cache->flags & term_flag) break;
		if(cache->flags & CACHE_FLAG_SUBSET) {
			//We'll get to it later
			subset_cache = cache;
		} else {
			if(subset_cache != NULL) {
				//
				// We've skipped issuing control functions to some cache levels
				// because they obey the 'subset' property. Have to issue one
				// to the last level now.
				//
				cache_one(subset_cache, base, len, flags);
			}
			cache_one(cache, base, len, flags);
			subset_cache = NULL;
		}
		cache_idx = cache->next;
	}
	if(subset_cache != NULL) {
		//
		// We've skipped issuing control functions to some cache levels
		// because they obey the 'subset' property. Have to issue one
		// to the last level now.
		//
		cache_one(subset_cache, base, len, flags);
	}
}

int
cache_control(uintptr_t base, size_t len, int flags) {
	struct cpuinfo_entry	*cpu;
	int						data_flags;
	int						code_flags;

	//NYI: deal with SMP issues
	cpu = &SYSPAGE_ENTRY(cpuinfo)[0];
	data_flags = flags & (MS_SYNC|MS_ASYNC|MS_INVALIDATE);
	if(data_flags != 0) {
		cache_iterate(cpu->data_cache, base, len, data_flags, 0);
	}
	code_flags = flags & (MS_INVALIDATE|MS_INVALIDATE_ICACHE);
	if(code_flags != 0) {
		if(data_flags == 0) {
			//Push data towards main memory until we run into a unified
			//cache. This is so that when the icache(s) refill, they'll
			//get up-to-date data.
			cache_iterate(cpu->data_cache, base, len, MS_SYNC, CACHE_FLAG_INSTR);
		}
		cache_iterate(cpu->ins_cache, base, len, code_flags, 0);
	}
	return(0);
}

void
cache_flush(uintptr_t base, size_t len) {
	cache_control(base, len, MS_SYNC|MS_INVALIDATE_ICACHE);
}
