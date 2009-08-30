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

struct sync_data {
	off64_t			off;
	struct mm_map	*mm;
	int				got;
	unsigned		bits;
	unsigned		adp_flags;
};

struct cache_data {
	uintptr_t		start;
	uintptr_t		next;
	unsigned		flags;
	ADDRESS			*as;
	struct mm_map	*mm;
};

static int
pte_off(struct mm_object_ref *or, struct mm_map *mm, void *d) {
	struct sync_data	*data = d;
	ADDRESS				*adp;
	unsigned			flags;
	struct mm_map		*sync_mm;
	off64_t				sync_start;
	off64_t				sync_end;
	off64_t				start;
	off64_t				end;
	uintptr_t			start_vaddr;
	uintptr_t			end_vaddr;

	adp = or->adp;
	data->adp_flags |= adp->flags;
	if(!(adp->flags & MM_ASFLAG_ISR_LOCK)) {
		flags = mm->mmap_flags;
		if(flags & data->bits) {
			sync_mm = data->mm;
			sync_start = sync_mm->offset;
			sync_end   = sync_start + (sync_mm->end - sync_mm->start);
			start      = mm->offset;
			end        = start + (sync_mm->end - sync_mm->start);
			if((start >= sync_start) && (start <= sync_end)
			||(end    >= sync_start) && (end   <= sync_end)) {
				// In range
				if(start < sync_start) start = sync_start;
				if(end   > sync_end)   end = sync_end;
				start_vaddr = mm->start + (uintptr_t)(start - mm->offset);
				end_vaddr   = mm->start + (uintptr_t)(end - mm->offset);
				if(data->bits & PROT_READ) {
					pte_unmap(adp, start_vaddr, end_vaddr, or->obp);
				} else {
					(void)pte_prot(adp, start_vaddr, end_vaddr, flags & ~data->bits, or->obp);
				}
			}
		}
	}
	return EOK;
}


static int
write_page(struct sync_data *data, void *buff) {
	OBJECT		*obp = data->mm->obj_ref->obp;
	off64_t		off = data->off;
	off64_t		len;
	int			got;

	len = obp->mem.mm.size - off;
	if(len > QUANTUM_SIZE) len = QUANTUM_SIZE;

	got = pwrite64(obp->fdmem.fd, buff, (unsigned)len, off);

	if((got >= 0) && (got != (unsigned)len)) {
		errno = ENOSPC;
		got = -1;
	}
	return got;
}


static int
do_write(void *dst, size_t len, void *d) {
	struct sync_data	*data = d;

#ifndef NDEBUG	
	// Things need to get more sophisticated if we write out more than 
	// a page at a time
	if(len != QUANTUM_SIZE) crash();
#endif	

	data->got = write_page(data, dst);
	return EOK;
}


static int
pmem_off(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct sync_data	*data = d;
	struct mm_map		*mm;
	uintptr_t			vaddr;
	unsigned			got;
	unsigned			i;
	unsigned			bits;

	for(i = 0; i < num; ++i) {
		//FUTURE: Gather up contiguous runs of PAQ_FLAG_MODIFIED
		//FUTURE: and write them out in one go.
		bits = data->bits;
		if((pq->flags & bits) == PAQ_FLAG_MODIFIED) {
			mm = data->mm;
			vaddr = mm->start + (uintptr_t)(off - mm->offset);
			data->off = off;
			// We can't use 'vaddr' directly to write out the page
			// because we can't guarantee that the page table is properly
			// set up (consider a second process does a msync() for the
			// object followed by a third touching the memory to turn the
			// PAQ_FLAG_MODIFIED back on).
			(void)pte_temp_map(mm->obj_ref->adp, vaddr, pq, mm, QUANTUM_SIZE, do_write, data);
			got = data->got;
			if(got == (unsigned)-1) return errno;
		}
		// If any aspaces referencing this memory is super-locked,
		// we can't fiddle the pa_quantum bits because we can't fiddle
		// the page table 'properly'.
		if(data->adp_flags & MM_ASFLAG_ISR_LOCK) bits = 0;
		pq->flags &= ~bits;
		++pq;
		off += QUANTUM_SIZE;
	}
	return EOK;
}


int
mm_sync(OBJECT *obp, struct mm_map *mm, int flags) {
	struct sync_data		data;
	off64_t					end;
	int						r;

	data.adp_flags = 0;
	data.mm = mm;
	end = mm->offset + (mm->end - mm->start);
	if(flags & (MS_SYNC|MS_ASYNC)) {
		data.bits = PROT_WRITE;
		memref_walk(obp, pte_off, &data);
		data.bits = PAQ_FLAG_MODIFIED;
		r = memobj_pmem_walk(MPW_SYSRAM, obp, mm->offset, end, pmem_off, &data);
		if(r != EOK) return r;
		if(flags & MS_SYNC) {
			if(fdatasync(obp->fdmem.fd) != 0) {
				return errno;
			}
		}
	}
	r = EOK;
	if(flags & MS_INVALIDATE) {
		data.bits = PROT_READ|PROT_WRITE|PROT_EXEC;
		memref_walk(obp, pte_off, &data);
		data.bits = PAQ_FLAG_INITIALIZED;
		r = memobj_pmem_walk(MPW_SYSRAM, obp, mm->offset, end, pmem_off, &data);
	}
	return r;
}


static void
one_cache(struct cache_data *cache, uintptr_t start, uintptr_t next) {
	if(cache->start == VA_INVALID) {
		cache->start = start;
	} else if(cache->next != start) {
		// Had a hole, need to do the cache op for previous piece
		CPU_CACHE_CONTROL(cache->as, (void *)cache->start, 
					cache->next - cache->start, cache->flags);
		cache->start = start;
	}
	cache->next = next;
}


static int
check_cache(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct cache_data	*cache = d;
	uintptr_t			start;

	start = (uintptr_t)(off - cache->mm->offset) + cache->mm->start;
	one_cache(cache, start, start + NQUANTUM_TO_LEN(num));
	return EOK;
}


int 
vmm_msync(PROCESS *prp, uintptr_t vaddr, size_t len, int flags) {
	ADDRESS					*as;
	int						r;
	struct map_set			ms;
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	OBJECT					*obp;
	struct cache_data		cache;

	/* MS_SYNC and MS_ASYNC are mutually exclusive */
	if ((flags & (MS_SYNC | MS_ASYNC)) == (MS_SYNC | MS_ASYNC))
		return EINVAL;

	as = prp->memory;
	r = map_isolate(&ms, &as->map, vaddr, len, MI_SPLIT);
	if(r != EOK) goto fail1;

	// We have to make sure there are extra threads available in case 
	// CacheControl() puts this thread into WAITPAGE, or when we
	// go to write a page out.
	if(proc_thread_pool_reserve() != 0) {
		r = EAGAIN;
		goto fail2;
	}

	// CacheControl() might cause page faults, so let fault_pulse() 
	// know that it doesn't have to grab the lock for this reference
	proc_lock_owner_mark(prp);


	if(flags & MS_ASYNC) {
		cache.flags = (flags & ~MS_ASYNC) | MS_SYNC;
	} else {
		cache.flags = flags;
	}
	cache.start = VA_INVALID;
	cache.as = as;
	for(mm = ms.first; mm != ms.last->next; mm = mm->next) {
		if(mm->mmap_flags & MAP_LAZY) {
			// We only want to do cache operations on memory that's been
			// allocated so that we avoid cache instructions 'touching'
			// non-existent pages and causing them to become allocated.
			or = mm->obj_ref;
			if(or != NULL) {
				unsigned has_lock;

				obp = or->obp;
				has_lock = memobj_cond_lock(obp);
				cache.mm = mm;
				(void)memobj_pmem_walk_mm(MPW_SYSRAM|MPW_PHYS, mm, check_cache, &cache);
				if(!has_lock) memobj_unlock(obp);
			}
		} else {
			// We could use memobj_pmem_walk_mm() for non-lazy regions
			// as well, but it's faster to do it in one gulp.
			one_cache(&cache, mm->start, mm->end + 1);
		}
	}
	if(cache.start != VA_INVALID) {
		CPU_CACHE_CONTROL(as, (void *)cache.start, cache.next - cache.start, 
				cache.flags);
	}

	if(!(flags & (MS_INVALIDATE_ICACHE|MS_CACHE_ONLY))) {
		for(mm = ms.first; mm != ms.last->next; mm = mm->next) {
			if((flags & MS_INVALIDATE) && (mm->extra_flags & EXTRA_FLAG_LOCK)) {
				r = EBUSY;
				goto fail3;
			}
			or = mm->obj_ref;
			if (or != NULL) {
				obp = or->obp;

				if ((flags & MS_INVALIDATE) ||
					((flags & (MS_SYNC | MS_ASYNC)) && !(mm->mmap_flags & MAP_NOSYNCFILE)))
				{
					unsigned has_lock = memobj_cond_lock(obp);
					if((obp->hdr.type == OBJECT_MEM_FD) &&
						((obp->mem.mm.flags & MM_FDMEM_NEEDSSYNC) || (flags & MS_INVALIDATE))) {
						/*
						 * if the caller specified MS_INVALIDATE on a NOSYNCFILE mapping,
						 * skip the MS_SYNC/MS_ASYNC processing
						*/
						int sync_flags = (mm->mmap_flags & MAP_NOSYNCFILE) ? flags & ~(MS_SYNC|MS_ASYNC) : flags;
						r = mm_sync(obp, mm, sync_flags);
					}
					if(!has_lock) memobj_unlock(obp);
					if(r != EOK) goto fail3;
				}
			}
		}
	}

fail3:
	proc_thread_pool_reserve_done();

fail2:	
	map_coalese(&ms);

fail1:	
	return r;
}

__SRCVERSION("vmm_msync.c $Rev: 199396 $");
