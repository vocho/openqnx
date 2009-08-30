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


struct data {
	off64_t				off_start;
	off64_t				off_end;
	struct mm_map		*mm;
	struct pa_quantum	*pq;
	uintptr_t			va;
	int					unlock_obp;
	int					icache_check;
	unsigned			flags;
};

#define INIT_MPW_FLAGS	MPW_SYSRAM	// Might be overridden - see pq_init().

//RUSH3: pq_purge_cache isn't going to do an awful lot of good on CPU's
//RUSH3: with virtually indexed & tagged caches (ARM). On those systems
//RUSH3: we have to have purged the cache on the unmap. We should skip
//RUSH3: this code (and the the gear in pq_map() that fiddles with PROT_NOCACHE)
//RUSH3: since the cpu_pte_manipulate() will have done the right thing
//RUSH3: already.

static int
pq_purge_cache(void *dst, size_t len, void *d) {
	CPU_CACHE_CONTROL(((struct data *)d)->mm->obj_ref->adp, dst, len, MS_INVALIDATE|MS_INVALIDATE_ICACHE);
	return EOK;
}


//RUSH3: Rework the interface to condition_cache() so that it's given
//RUSH3: a vaddr to work with - avoids the pte_temp_map() in the
//RUSH3: non-colour case. In two out of the three times it's called, we
//RUSH3: turn around and to a pte_temp_map() immediately following
static int rdecl
condition_cache(struct mm_map *mm, struct pa_quantum *pq, unsigned num, struct data *data) {
	int		r;

	// If PROT_NOCACHE is on, we have to purge any cache entries 
	// for the physical memory that might be lurking about.
	
#if CPU_SYSTEM_HAVE_COLOURS		
	if(mm->mmap_flags & PROT_NOCACHE) {
		if(pq->blk != PAQ_BLK_FAKE) {
			unsigned remaining = num;

			do {
				if(PAQ_GET_COLOUR(pq) != PAQ_COLOUR_NONE) {
					cpu_colour_clean(pq, COLOUR_CLEAN_PURGE);
					PAQ_SET_COLOUR(pq, PAQ_COLOUR_NONE);
				}
				++pq;
			} while(--remaining != 0);
			pq -= num;
		}
		if(mm_flags & MM_FLAG_MULTIPLE_DCACHE_LEVELS) {
			mm->mmap_flags &= ~PROT_NOCACHE;
			r = pte_temp_map(mm->obj_ref->adp, data->va, pq, mm, num << QUANTUM_BITS, pq_purge_cache, data);
			mm->mmap_flags |= PROT_NOCACHE;
			if(r != EOK) return r;
		}
	} else {
		colour_set(data->va, pq, num);
	}
#else
	if(mm->mmap_flags & PROT_NOCACHE) {
		mm->mmap_flags &= ~PROT_NOCACHE;
		r = pte_temp_map(mm->obj_ref->adp, data->va, pq, mm, num << QUANTUM_BITS, pq_purge_cache, data);
		mm->mmap_flags |= PROT_NOCACHE;
		if(r != EOK) return r;
	}
#endif
	return EOK;
}


static int
pq_init_run(void *dst, size_t len, void *d) {
	struct data			*data = d;
	struct mm_map		*mm;
	struct pa_quantum	*pq;
	OBJECT				*obp;
	size_t				got;
	unsigned			last_page_bss;
	uintptr_t			end;
	uintptr_t			real_end;

	pq = data->pq;
	mm = data->mm;
	obp = mm->obj_ref->obp;
	//RUSH3: Make more object oriented...
	switch(obp->hdr.type) {
	case OBJECT_MEM_TYPED:
	case OBJECT_MEM_ANON:
	case OBJECT_MEM_SHARED:
		if(mm->mmap_flags & PROT_NOCACHE) {
			CPU_ZERO_PAGE(dst, len, mm);
		} else {
			void *endp;

			endp = (uint8_t *)dst + len;
			do {
				if(!(pq->flags & PAQ_FLAG_ZEROED)) {
					CPU_ZERO_PAGE(dst, QUANTUM_SIZE, mm);
					//RUSH3: If we set PAQ_FLAG_ZEROED here and maintained
					//RUSH3: it properly (e.g. turned it off when setting 
					//RUSH3: PAQ_FLAG_MODIFIED), we might be able save
					//RUSH3: some zeroing when reusing the page. Gather
					//RUSH3: some stats on how often a page is freed while
					//RUSH3: still zeroed - don't forget to turn off
					//RUSH3: PAQ_FLAG_ZEROED in the OBJECT_MEM_FD case.
				}
				++pq;
				dst = (uint8_t *)dst + QUANTUM_SIZE;
			} while(dst < endp);
		}
		break;
	case OBJECT_MEM_FD:	
		if(proc_thread_pool_reserve() != 0) {
			return EAGAIN;
		}
		end = data->va + len - 1;
		real_end = mm->end - mm->last_page_bss;
		if(end > real_end) {
			last_page_bss = end - real_end;
			mm->last_page_bss -= last_page_bss;
		} else {
			last_page_bss = 0;
		}
		got = proc_read(obp->fdmem.fd, dst, len - last_page_bss, 
						mm->offset + (data->va - mm->start));
		proc_thread_pool_reserve_done();
		if(got == (size_t)-1) return errno;
		if((len != got) && !(pq->flags & PAQ_FLAG_ZEROED)) {
			// Partial last page...
			memset((char *)dst + got, 0, len - got);
		}
		data->icache_check = 1;
		break;
	default:
		crash();
	}

	data->va += len;
	return EOK;
}


//RUSH1: Have to be more careful here. If one guy has the page
//RUSH1: mapped MAP_NOINIT and another doesn't, we have a race
//RUSH1: condition about whether the page is inited or not.
//RUSH1: Can only do the MAP_NOINIT if _all_ mmap's to this
//RUSH1: page are MAP_NOINIT.
#define	INIT_REQUIRED(mmf, pq)					\
	(!((pq)->flags & PAQ_FLAG_INITIALIZED)		\
	  && (((pq)->flags & PAQ_FLAG_INIT_REQUIRED) || !((mmf) & MAP_NOINIT)))


static int
pq_init(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct data		*data = d;
	int				r;
	struct mm_map	*mm;
	unsigned		i;

#ifndef NDEBUG	
	// Check to make sure we've allocated pmem for the whole range
	if(pq == NULL) crash();
	#undef INIT_MPW_FLAGS
	#define INIT_MPW_FLAGS	(MPW_HOLES|MPW_SYSRAM)
#endif	
	mm = data->mm;
	
	for( ;; ) {
		// skip over pages that don't need initialization
		for( ;; ) {
			if(num == 0) return EOK;
			if(INIT_REQUIRED(mm->mmap_flags, pq)) break;
			if(!(pq->flags & PAQ_FLAG_INITIALIZED)) {
				r = condition_cache(mm, pq, 1, data);
				if(r != EOK) return r;
				pq->flags = (pq->flags & ~PAQ_FLAG_INIT_REQUIRED) | PAQ_FLAG_INITIALIZED;
			}
			++pq;
			data->va += QUANTUM_SIZE;
			--num;
		}
		// find the run of pages that need initialization;
		data->pq = pq;
		i = 0;
		for( ;; ) {
			pq->flags = (pq->flags & ~PAQ_FLAG_INIT_REQUIRED) | PAQ_FLAG_INITIALIZED;
			++pq;
			++i;
			if(i >= num) break;
			if(!INIT_REQUIRED(mm->mmap_flags, pq)) break;
		}
		// do the initialization
		r = condition_cache(mm, data->pq, i, data);
		if(r != EOK) return r;
		r = pte_temp_map(mm->obj_ref->adp, data->va, data->pq, mm, i << QUANTUM_BITS, pq_init_run, d);
		if(r != EOK) return r;
		num -= i;
	}
}


static int
pq_map(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct data				*data = d;
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	uintptr_t				start;
	uintptr_t				curr;
	paddr_t					paddr;
	off64_t					skip;
	unsigned				mmap_flags;
	int						r;
	unsigned				i;
	unsigned				paq_flag_intersection;
	unsigned				mask;
	unsigned				map_type;
	int						need_dcache_flush_for_icache;

	mm = data->mm;
	or = mm->obj_ref;
	mmap_flags = mm->mmap_flags;
	map_type = mmap_flags & MAP_TYPE;
	start = data->va;
	paddr = pa_quantum_to_paddr(pq);
	need_dcache_flush_for_icache = 0;
	if(pq->blk == PAQ_BLK_FAKE) {
		//We only have a single pa_quantum_fake to cover the whole
		//of the run, so we need to adjust the paddr with the
		//offset from the start.
		skip = 0;
		if(data->off_start > off) {
			skip = data->off_start - off;
			paddr += skip;
			num -= LEN_TO_NQUANTUM(skip);
		}
		if((map_type != MAP_SHARED) && !(data->flags & MR_WRITE)) {
			// Still want to fault on first write
			mmap_flags &= ~PROT_WRITE;
		}
#if CPU_SYSTEM_HAVE_COLOURS		
		if(!(pq->flags & PAQ_FLAG_INITIALIZED) && !(mmap_flags & PROT_NOCACHE)) {
			colour_set(start - skip, pq, 1);
			pq->flags |= PAQ_FLAG_INITIALIZED;
		}
#endif		
		if(!(mm->extra_flags & EXTRA_FLAG_CACHE_CLEANED)) {
			mm->extra_flags |= EXTRA_FLAG_CACHE_CLEANED;
			// We're making an assumption here that we set up the PTE's
			// for a direct mapping in only one operation and as such
			// we can get away with just maintaining one bit about the
			// cache being invalidated rather than a bitmask for each
			// of the pages. This is currently the way things work
			// because of the gear in vmm_mmap.c (setup_mappings()). If
			// things ever change, the check below will catch the
			// problem and someone will read this comment and say
			// Aaarrrggghhh!
			if((((mm->end - mm->start) + 1) != NQUANTUM_TO_LEN(num)) 
				&& (obp->hdr.type == OBJECT_MEM_ANON)) {
				crash();
			}
			//RUSH3: We can optimize this by not turning off the PROT_NOCACHE
			//RUSH3: if the paddr doesn't point to RAM: pa_paddr_to_quantum()
			//RUSH3: returns NULL. Maybe not - what about special purpose
			//RUSH3: ram that's outside the sysram region(s)?
			// We need to map cached initially so we can flush
			mmap_flags &= ~PROT_NOCACHE;
		}

		data->va = start + NQUANTUM_TO_LEN(num);
		r = pte_map(or->adp, start, data->va - 1, mmap_flags, or->obp, paddr, 0);
		if(r == EOK) {
			if(mmap_flags & PROT_WRITE) {
				pq->flags |= PAQ_FLAG_MODIFIED;
			}
			if(!(mmap_flags & PROT_NOCACHE) && (mm->mmap_flags & PROT_NOCACHE)) {
				// We've done a cached mapping, but the actual flags were for
				// no-cache. We did this so that we can make sure there are
				// no bits sitting around in the cache for the physical memory
				// by invalidating the cache first
				CPU_CACHE_CONTROL(or->adp, (void *)start, data->va - start, MS_INVALIDATE|MS_INVALIDATE_ICACHE);
				r = pte_map(or->adp, start, data->va - 1, mmap_flags | PROT_NOCACHE, or->obp, paddr, 0);
			}
		}
	} else { 
		data->va = start + NQUANTUM_TO_LEN(num);
		curr = start;
		for( ;; ) {
			// Skip over non-initialized regions
			for( ;; ) {
				if(num == 0) return EOK;
				if(pq->flags & PAQ_FLAG_INITIALIZED) break;
				if(mmap_flags & PROT_EXEC) {
					pq->flags |= PAQ_FLAG_HAS_INSTRS;
				}
				++pq;
				curr += QUANTUM_SIZE;
				paddr += QUANTUM_SIZE;
				--num;
			}
			paq_flag_intersection = ~0;
			start = curr;
			// Find run of init'd pages
			i = 0;
			for( ;; ) {
#if CPU_SYSTEM_HAVE_COLOURS		
				if(obp->mem.mm.flags & MM_MEM_COLOUR_FLUSH) {
					// We've set up multiple mappings to the same memory with
					// different colours. Normally that's a no-no, but in some
					// cases it's OK - e.g. running a program from "/dev/shmem".
					// We know that we're only going to be reading from the
					// mapping, so it's OK if it ends up in multiple places in
					// the data cache. We do, however, have to make sure that
					// the pages are all flushed to main memory so the cache
					// aliases pick up the right stuff.
					unsigned	pq_colour = PAQ_GET_COLOUR(pq);
					if((pq_colour != PAQ_COLOUR_NONE) && (pq_colour != COLOUR_VA(curr))) {
						cpu_colour_clean(pq, COLOUR_CLEAN_FLUSH);
					}
				}
#endif
				paq_flag_intersection &= pq->flags;
				if(data->flags & MR_WRITE) {
					pq->flags |= PAQ_FLAG_MODIFIED;
				}
				if(!(mmap_flags & PROT_EXEC) && data->icache_check && (pq->flags & PAQ_FLAG_HAS_INSTRS)) {
					pq->flags &= ~PAQ_FLAG_HAS_INSTRS;
					need_dcache_flush_for_icache = 1;
				}
				if(mm->extra_flags & EXTRA_FLAG_LOCK) {
					//If the mapping has the region locked, make sure
					//the pmem flag is on as well (might not be if MAP_LAZY)
					pq->flags |= PAQ_FLAG_LOCKED;
				}
				curr += QUANTUM_SIZE;
				++pq;
				++i;
				if(i == num) break;
				if(!(pq->flags & PAQ_FLAG_INITIALIZED)) break;
			}
			if(data->flags & MR_WRITE) {
				mask = ~0;
			} else if((paq_flag_intersection & PAQ_FLAG_MODIFIED) && (map_type == MAP_SHARED)) {
				// If MAP_SHARED and all the pages were already marked as
				// modified, we can turn on write perms in the PTE, even
				// if this reference is only a read request.
				mask = ~0;
			} else {
				// Still need to know about the first write to the pages.
				mask = ~PROT_WRITE;
			}
			// Map the initialized region
			r = pte_map(or->adp, start, curr - 1, mmap_flags & mask, or->obp, paddr, 0);
			if(r != EOK) break;
			paddr += i << QUANTUM_BITS;
			num -= i;
			if(data->icache_check) {
				unsigned	cache_flags = 0;

				if(mm->mmap_flags & PROT_EXEC) {
					cache_flags = MS_INVALIDATE_ICACHE;
				} else if(need_dcache_flush_for_icache) {
					// We don't have PROT_EXEC perms for the mapping, but there's
					// at least one page that has a different mapping that does
					// have PROT_EXEC and wasn't initialized at the time of the
					// mmap(). We need to make sure that we flush the page down
					// to a unified cache so the other mapping will pick up the
					// right data for instruction fetches
					cache_flags = MS_ASYNC;
				}
				if(cache_flags != 0) {
					//RUSH3: If CPU_CACHE_CONTROL returns that it did
					//RUSH3: a full flush, we can skip any further
					//RUSH3: cache operations for this memory_reference().
					//RUSH3: Things to think about: MS_INVALIDATE_ICACHE
					//RUSH3: vs MS_ASYNC, privatize() mapping vs final
					//RUSH3: mapping in memory_reference().
					CPU_CACHE_CONTROL(or->adp, (void *)start, curr - start, cache_flags);
					//RUSH1: There's a period of time after we've mapped the pages but 
					//RUSH1: before we've flushed. If another thread started executing 
					//RUSH1: on the page in that interval, it could get messed up (think
					//RUSH1: SMP or this code getting preempted). 
					//RUSH1: We could:
					//RUSH1:     - do the CacheControl on a temp mapping (bad for virt caches) 
					//RUSH1:     - map with kernel access priv only until after the
					//RUSH1:       flush (maybe not possible everywhere)
					//RUSH1:     - hold off anybody who's running/attempting to run
					//RUSH1:       with the aspace (doing this might allow us to
					//RUSH1:       use pte_temp_map() less).
				}
			}
		}
	}
	return r;
}


static int
pq_copy_run(void *dst, size_t len, void *d) {
	struct data		*data = d;
	struct mm_map	*mm;
	size_t			last_page_bss;
	uintptr_t		src;

	mm = data->mm;
	last_page_bss = mm->last_page_bss;
	src = data->va;
	data->va += len;
	if((data->va-1) > (mm->end - last_page_bss)) {
		len -= last_page_bss;
		memset((char *)dst + len, 0, last_page_bss);
	}
	memcpy(dst, (void *)src, len);
	return EOK;
}


static int
pq_copy(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct data		*data = d;
	struct mm_map	*mm;
	int				r;

	mm = data->mm;
	r = condition_cache(mm, pq, num, data);
	if(r == EOK) {
		r = pte_temp_map(mm->obj_ref->adp, data->va, pq, mm, NQUANTUM_TO_LEN(num), pq_copy_run, d);
		if(r == EOK) {
			do {
				pq->flags |= PAQ_FLAG_INITIALIZED;
				++pq;
				--num;
			} while(num > 0);
		}
	}
	return r;
}


static int
privatize(struct data *data) {
	struct map_set			ms;
	struct map_set			repl_ms;
	int						r;
	uintptr_t				start;
	struct mm_object_ref	*or;
	OBJECT					*anon;
	OBJECT					*obp;
	struct mm_map			*mm;
	struct mm_map			*new;
	off64_t					off;
	size_t					len;
	ADDRESS					*adp;
	unsigned				orig_mmap_flags;

	start = data->va;
	mm = data->mm;
	or = mm->obj_ref;
	adp = or->adp;
	obp = or->obp;


	//Create a readable mapping. We turn off icache_check so that
	//pq_map() doesn't bother with any cache flushing, since we're not going
	//to actually be executing from this storage. It gets turned on at the
	//end of this function. We also turn of MR_WRITE since we're not
	//actually modifying these pages
	//We temporarily set PROT_READ to ensure that the mapping is readable.
	//eg. if the mmap() specified only PROT_WRITE, the pq_map() will clear
	//the PROT_WRITE to catch the first write, resulting in a crash in the
	//pq_copy() below because the mapping will have no access permissions.
	data->icache_check = 0;
	data->flags &= ~MR_WRITE;
	orig_mmap_flags = mm->mmap_flags;
	mm->mmap_flags |= PROT_READ;
	r = memobj_pmem_walk(MPW_PHYS|MPW_SYSRAM, obp, data->off_start, data->off_end, pq_map, data);
	mm->mmap_flags = orig_mmap_flags;
	if(r != EOK) goto fail1;
	data->flags |= MR_WRITE;

	// Allocate anonymous memory
	anon = adp->anon;
	if(ANMEM_MULTI_REFS(anon)) {
		memobj_lock(anon);
	}
	len = (data->off_end - data->off_start) + 1;
	//RUSH2: Assuming that we always get the same offset for the same 'start'
	off = anmem_offset(anon, start, len);
	if(mm->mmap_flags & MAP_LAZY) {
		r = memobj_pmem_add(anon, off, len, mm->mmap_flags);
		if(r != EOK) goto fail2;
	}
	// Copy from the shared to private storage
	data->va = start;
	data->off_start = off;
	data->off_end = off + len - 1;
	memobj_pmem_walk(MPW_SYSRAM, anon, off, data->off_end, pq_copy, data);
	//Switch the map structures to the anonymous memory
	r = map_create(&ms, &repl_ms, &adp->map, start, len, 0, MAP_FIXED);
	if(r != EOK) goto fail3;
	new = ms.first;
	new->offset = off;
//START KLUDGE
	//FUTURE: We'd like to turn off the MAP_ELF here, since that will make it
	//FUTURE: more likely that we'll be able to merge this mm_mmap with one
	//FUTURE: following. Unfortunately, this region could be a PLT on the PPC,
	//FUTURE: and the libc ldd code is going to turn around and issue an
	//FUTURE: mprotect() for it to invalidate the instruction cache. In old 
	//FUTURE: libc's, that mprotect isn't going to be page aligned like it 
	//FUTURE: should, so the new memmgr will fail it unless the MAP_ELF flag 
	//FUTURE: is on :-(. See memmgr_ctrl.c for details.
#if 0
	new->mmap_flags = (mm->mmap_flags & ~(MAP_PHYS|MAP_ELF|MAP_NOSYNCFILE)) | MAP_SYSRAM | MAP_ANON;
#else	
	new->mmap_flags = (mm->mmap_flags & ~(MAP_PHYS|MAP_NOSYNCFILE)) | MAP_SYSRAM | MAP_ANON;
#endif	
//END KLUDGE
	new->extra_flags = mm->extra_flags;
	new->reloc = mm->reloc;
	//MAPFIELDS: copy field data from 'mm' to 'new'
	adp->flags |= MM_ASFLAG_PRIVATIZING;
	r = ms_unmap(adp, &repl_ms, UNMAP_PRIVATIZE|UNMAP_NORLIMIT);
	if(r != EOK) goto fail4;
	r = map_add(&ms);
	adp->flags &= ~MM_ASFLAG_PRIVATIZING;
	if(r != EOK) goto fail4;
	r = memref_add(new, anon, adp, NOFD);
	if(r != EOK) goto fail5;
	map_coalese(&ms);

	data->mm = ms.first;
	data->unlock_obp = ANMEM_MULTI_REFS(anon);
	data->icache_check = 1;
	return EOK;

fail5:
	map_remove(&ms);

fail4:
	map_destroy(&ms);
	map_coalese(&repl_ms);
	adp->flags &= ~MM_ASFLAG_PRIVATIZING;

fail3:
	if(mm->mmap_flags & MAP_LAZY) {
		memobj_pmem_del_len(anon, off, len);
	}

fail2:
	if(mm->mmap_flags & MAP_LAZY) {
		anmem_unoffset(anon, off, len);
	}
	if(ANMEM_MULTI_REFS(anon)) {
		memobj_unlock(anon);
	}

fail1:
	return r;
}


int
memory_reference(struct mm_map **mmp, uintptr_t start, uintptr_t end, 
		unsigned flags, struct map_set *ms) {
	OBJECT					*obp;
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	int						r;
	struct data				data;
	uintptr_t				valid_end;

	mm = *mmp;
	//RUSH2: When we allocate pmem, we need to check if 
	//RUSH2: freespace < some_limit(s) and, if so, start
	//RUSH2: paging out globally. If mm_aspace->rlimit.rss > RLIMIT_RSS, start
	//RUSH2: paging out the process.

	//Only handles one mm_map entry at a time.
	CRASHCHECK(start < mm->start || end > mm->end);

	or = mm->obj_ref;
	obp = or->obp;
	
	if(mm->mmap_flags & PROT_WRITE) {
		if(!CPU_FAULT_ON_WRITE_WORKS || (or->adp->flags & MM_ASFLAG_ISR_LOCK)) {
			// If we can write to this memory and fault on write doesn't
			// work (386) or the aspace is super-locked, always pretend the
			// access is a write to force privatization (if needed)
			flags |= MR_WRITE;
			//RUSH3: If we force on MR_WRITE and MR_NOINIT is also on,
			//RUSH3: kick out of the optimization because privatize() doesn't 
			//RUSH3: handle a range that has uninitialized quantums properly.
			if(((mm->mmap_flags & MAP_TYPE) == MAP_PRIVATE) 
				&& (obp->hdr.type != OBJECT_MEM_ANON) 
				&& (flags & MR_NOINIT)) {
				return EOK;
			}
		}
	}

	//RUSH1: We might be asked to 'reference' beyond the end of a shared
	//RUSH1: memory object (the start/end pointers might be beyond 
	//RUSH1: obp->mem.mm.size). Make sure we handle that without blowing
	//RUSH1: up.
	start = ADDR_PAGE(start);
	end = ADDR_PAGE(end) | (__PAGESIZE-1);
	data.off_end   = mm->offset + (end - mm->start);
	data.off_start = mm->offset + (start - mm->start);
	if(flags & MR_TRUNC) {
		// truncate reference to the limit of the object
		if(data.off_start >= obp->mem.mm.size) {
			// first reference is past end of object - user will fault if he
			// ends up touching this location
			return EOK;
		}
		if(data.off_end >= obp->mem.mm.size) {
			data.off_end = (obp->mem.mm.size-1) | (__PAGESIZE-1);
		}
	}

	data.mm = mm;
	data.unlock_obp = 0;
	data.icache_check = 0;
	data.flags = flags;

	valid_end = mm->end - mm->last_page_bss;

	if(!(flags & MR_NOINIT)) {
		if((mm->mmap_flags & MAP_LAZY) || SHM_LAZY(obp)) {
			r = memobj_pmem_add(obp, data.off_start, (end - start) + 1, mm->mmap_flags);
			if(r != EOK) return r;
		}
		data.va = start;
		r = memobj_pmem_walk(INIT_MPW_FLAGS, obp, data.off_start, data.off_end, pq_init, &data);
		if(r != EOK) return r;
		if(end > valid_end) {
			// Force privatization. We can only zero the last partial page
			// when it's been privatized, since the memory might be coming from
			// an OBJECT_MEM_SHARED and we don't want to damage the underlying
			// memory.
			flags |= MR_WRITE;
		}
	} else if(end > valid_end) {
		// Don't try to map in the partial last page - we need to
		// wait until we really reference it so it can be privatized
		if((data.off_end-data.off_start) <= QUANTUM_SIZE) {
			return EOK;
		}
		data.off_end -= QUANTUM_SIZE;
	}

	//RUSH2: Have to handle COW, COR 
	if(flags & MR_WRITE) {
		// MR_WRITE might have been turned on after first assignment above
		data.flags = flags; 
		if((mm->mmap_flags & MAP_TYPE) == MAP_PRIVATE) {
			switch(obp->hdr.type) {
			case OBJECT_MEM_ANON:	
				break;
			default:	
				// Change the memory to private and anonymous.
				data.va = start;
				r = privatize(&data);
				if(r != EOK) return r;
				if(mm != data.mm) {
					if(ms != NULL) {
						map_set_update(ms, mm, data.mm);
					}
					mm = data.mm;
					*mmp = mm;
					or = mm->obj_ref;
					obp = or->obp;
				}
				break;
			}
		} else if(obp->hdr.type == OBJECT_MEM_FD) {
			if(!(mm->mmap_flags & MAP_NOSYNCFILE)) {
				obp->mem.mm.flags |= MM_FDMEM_NEEDSSYNC;
			}
		}
	}

	switch(obp->hdr.type) {
	case OBJECT_MEM_SHARED:
		if(obp->mem.mm.flags & MM_SHMEM_IFS) break;
		// fall through
	case OBJECT_MEM_FD:
	case OBJECT_MEM_TYPED:
		//RUSH3: If we could tell if the object has been written to
		//RUSH3: or not since the last PROC_EXEC mapping, we could
		//RUSH3: reduce the number of times that we have to invalidate
		//RUSH3: the icache.
		data.icache_check = 1;
		break;
	default:
		break;
	}

	data.va = start;
	r = memobj_pmem_walk(MPW_PHYS|MPW_SYSRAM, obp, data.off_start, data.off_end, 
							pq_map, &data);
	if(r != EOK) goto fail1;

fail1:	
	if(data.unlock_obp) memobj_unlock(obp);
	return r;
}

__SRCVERSION("mm_reference.c $Rev: 211761 $");
