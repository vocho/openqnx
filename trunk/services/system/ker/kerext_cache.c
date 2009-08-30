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

struct kerargs_cache {
	struct cacheattr_entry	*cache;
	uintptr_t				base;
	uintptr_t				end;
	int						flags;
	int						full;
};

//
// This can take a while, so we do it with the kernel unlocked and
// everything in a passed-in structure so that we can continue from
// where we left off if the kernel call gets preempted.
//
static void
kerext_cache_one(void *data) {
	struct kerargs_cache	*info = data;
	size_t					cache_len;
	struct cacheattr_entry	*cache;
	unsigned				flags;
	union {
		unsigned			(*_32)(_Paddr32t, unsigned, int, struct cacheattr_entry *, volatile struct syspage_entry *);
		unsigned			(*_64)(_Paddr64t, unsigned, int, struct cacheattr_entry *, volatile struct syspage_entry *);
	}						control;

	cache = info->cache;
	control._32 = cache->control;
	flags = cache->flags;
	info->base &= ~(cache->line_size-1);
	cache_len = info->end - info->base;
	while(cache_len != 0) {
		unsigned	lines;
		paddr_t		addr;
		size_t		valid_len;

		addr = info->base;
		valid_len = cache_len;
		if(flags & (CACHE_FLAG_CTRL_PHYS|CACHE_FLAG_CTRL_PHYS64)) {
			if(memmgr.vaddrinfo(aspaces_prp[KERNCPU], addr, &addr, &valid_len, VI_PGTBL) == PROT_NONE) {
				unsigned page = privateptr->pagesize;

				/* make vaddr skip to the next page boundary */
				addr = (info->base + page) & (~(page-1));
				if(cache_len < (addr - info->base)) {
					cache_len = 0;
				} else {
					cache_len -= (addr - info->base);
					info->base = addr;
				}
				continue;
			}
			if(valid_len > cache_len) valid_len = cache_len;
		}
		lines = (valid_len + cache->line_size - 1) / cache->line_size;
		while(lines != 0) {
			unsigned	done_lines;
			size_t		done_size;

			if(flags & CACHE_FLAG_CTRL_PHYS64) {
				done_lines = control._64((paddr64_t)addr, lines, info->flags, cache, _syspage_ptr);
			} else {
				done_lines = control._32((paddr32_t)addr, lines, info->flags, cache, _syspage_ptr);
			}
			if(done_lines == 0) {
				/* whole cache handled */
				info->full = 1;
				return;
			}
			done_size = done_lines * cache->line_size;
			info->base += done_size;
			addr += done_size;
			lines -= done_lines;
		}
		cache_len -= valid_len;
	}
}

static int
cache_one(struct cacheattr_entry *cache, void *base, size_t len, int flags) {
	struct kerargs_cache	info;
	unsigned				todo_runmask;
	unsigned				reset_runmask;
	int						full_flush;

	info.cache = cache;
	info.flags = flags;
	info.base = (uintptr_t)base;
	info.end = (uintptr_t)base + len;
	info.full = 0;

	if(am_inkernel()) {
		lock_kernel();
		kerext_cache_one(&info);
		return info.full;
	}

	reset_runmask = 0;
	todo_runmask = 0;
#if defined(VARIANT_smp)	
	if((NUM_PROCESSORS > 1) && (cache->flags & (CACHE_FLAG_NONCOHERENT|CACHE_FLAG_NOBROADCAST))) {
		todo_runmask = 1 << (NUM_PROCESSORS - 1);
		reset_runmask = LEGAL_CPU_BITMASK;
		noncoherent_caches = 1;
	}
#endif	
	do {
#if defined(VARIANT_smp)	
		if(todo_runmask != 0) {
			(void) ThreadCtl(_NTO_TCTL_RUNMASK, (void *)todo_runmask);
			todo_runmask >>= 1;
		}
#endif		
		full_flush = 1;
		do {
			if ( __Ring0(kerext_cache_one, &info) == -1 && errno == EFAULT )
				break;
			full_flush &= info.full;
		} while(!info.full && (info.base < info.end));
	} while(todo_runmask != 0);
#if defined(VARIANT_smp)	
	if(reset_runmask != 0) {
		(void) ThreadCtl(_NTO_TCTL_RUNMASK, (void *)reset_runmask);
	}
#endif	
	return full_flush;
}



static int
cache_iterate(int cache_idx, void *base, size_t len, int flags, unsigned term_flag) {
	struct cacheattr_entry	*cache_base;
	struct cacheattr_entry	*cache;
	struct cacheattr_entry	*subset_cache;
	int						full_flush;

	cache_base = SYSPAGE_ENTRY(cacheattr);
	full_flush = 1;
	subset_cache = NULL;
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
				full_flush &= cache_one(subset_cache, base, len, flags);
			}
			full_flush &= cache_one(cache, base, len, flags);
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
		full_flush &= cache_one(subset_cache, base, len, flags);
	}
	return(full_flush);
}

int
CacheControl(void *base, size_t len, int flags) {
	struct cpuinfo_entry	*cpu;
	int						data_flags;
	int						code_flags;
	int						full_flush = 0;

	full_flush = 1;
	cpu = &SYSPAGE_ENTRY(cpuinfo)[KERNCPU];
	data_flags = flags & (MS_SYNC|MS_ASYNC|MS_INVALIDATE);
	if(data_flags != 0) {
		// If MS_ASYNC is specified, only push the data out to the
		// first unified cache.
		full_flush &= cache_iterate(cpu->data_cache, base, len, 
				data_flags, (data_flags & MS_ASYNC) ? CACHE_FLAG_INSTR : 0);
	}
	code_flags = flags & (MS_INVALIDATE|MS_INVALIDATE_ICACHE);
	if(code_flags != 0) {
		if(data_flags == 0) {
			//Push data towards main memory until we run into a unified
			//cache. This is so that when the icache(s) refill, they'll
			//get up-to-date data.
			full_flush &= cache_iterate(cpu->data_cache, base, len, MS_SYNC, CACHE_FLAG_INSTR);
		}
		full_flush &= cache_iterate(cpu->ins_cache, base, len, code_flags, CACHE_FLAG_DATA);
	}

	return(full_flush);
}

__SRCVERSION("kerext_cache.c $Rev: 153052 $");
