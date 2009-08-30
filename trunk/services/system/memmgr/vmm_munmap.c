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


struct unmap_data {
	PROCESS			*prp;
	struct mm_map	*mm;
	unsigned		flags;
	int				unmap_flags;
	unsigned		check_limit;
	off64_t			start;
	off64_t			end;
};
	
static int
clean_pmem(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct unmap_data	*data = d;
	paddr_t				paddr;
	unsigned			skip;
	unsigned			sync_off;
	struct mm_map		*mm;
	uintptr_t			vaddr;

	mm = data->mm;
	paddr = pa_quantum_to_paddr(pq);
	if(pq->blk == PAQ_BLK_FAKE) {
		// If we've never granted write permission to any pages
		// in the mapping, we can't possibly have a sync object
		// in the region
		if(pq->flags & PAQ_FLAG_MODIFIED) {
			//We only have a single pa_quantum_fake to cover the whole
			//of the run, so we need to adjust the paddr with the
			//offset from the start.
			if(off < mm->offset) {
				skip = mm->offset - off;
				paddr += skip;
				num -= LEN_TO_NQUANTUM(skip);
				off = mm->offset;
			}
			sync_off = PADDR_TO_SYNC_OFF(paddr);
			MemobjDestroyed(PADDR_TO_SYNC_OBJ(paddr),
					sync_off, sync_off + ((num << QUANTUM_BITS) - 1), 
					data->prp, (void *)(mm->start + (uintptr_t)(off - mm->offset)));
		}
		data->flags |= PAQ_FLAG_INITIALIZED | PAQ_FLAG_INUSE;
	} else {
		vaddr = mm->start + (uintptr_t)(off - mm->offset);
		do {
			unsigned			pq_flags;

			pq_flags = pq->flags;
			if(data->unmap_flags & UNMAP_INIT_REQUIRED) {
				pq->flags = pq_flags | PAQ_FLAG_INIT_REQUIRED;
			}
			data->flags |= pq_flags | PAQ_FLAG_INUSE;
			if(pq_flags & PAQ_FLAG_HAS_SYNC) {
				PROCESS *prp = data->prp;

				sync_off = PADDR_TO_SYNC_OFF(paddr);
				MemobjDestroyed(PADDR_TO_SYNC_OBJ(paddr),
					sync_off, sync_off + (QUANTUM_SIZE - 1), 
					prp, (void *)vaddr);
			}
			vaddr += QUANTUM_SIZE;
			paddr += QUANTUM_SIZE;
			++pq;
		} while(--num != 0);
	}
	return EOK;
}

int
check_last_ref(struct mm_object_ref *or, struct mm_map *mm, void *d) {
	struct unmap_data		*data = d;
	off64_t					mm_start;
	off64_t					mm_end;

	if(data->mm == mm) return EOK;
	if(--data->check_limit == 0) {
		// We're taking too long, just bail out.
		data->start = data->end + 1;
		return -1;
	}
	mm_start = mm->offset;
	mm_end   = mm->offset + (mm->end - mm->start);
	//RUSH3: These tests for overlapping offsets can be more sophisticated
	if(mm_start > data->end) return EOK;
	if(mm_end < data->start) return EOK;
	if(mm_start < data->end ) data->end = mm_start;
	if(mm_end > data->start ) data->start = mm_end;
	if(data->start > data->end) return -1;
	return EOK;
}


int
ms_unmap(ADDRESS *adp, struct map_set *ms, int unmap_flags) {
	struct mm_map			*mm;
	struct mm_map			*end_mm;
	struct mm_object_ref	*or;
	struct unmap_data		data;
	OBJECT					*obp;
	int						haslock;
	int						has_unmapped = 0;

	CRASHCHECK(adp == NULL);
	data.prp =  object_from_data(adp, address_cookie);
	
	proc_lock_owner_mark(data.prp);

	if(!(unmap_flags & ~(UNMAP_PRIVATIZE|UNMAP_NORLIMIT))) {
		unmap_flags |= munmap_flags_default;
	}
	data.unmap_flags = unmap_flags;
	end_mm = ms->last->next;
	for(mm = ms->first; mm != end_mm; mm = mm->next) {
		if(mm->extra_flags & EXTRA_FLAG_SPECIAL) {
			if(!(data.prp->flags & (_NTO_PF_LOADING|_NTO_PF_TERMING))) {
				ms->flags |= MI_SKIP_SPECIAL;
				continue;
			}
		}
		has_unmapped = 1;
		if(!(data.unmap_flags & UNMAP_NORLIMIT)) {
			size_t		size;

			size = (mm->end - mm->start) + 1;
			adp->rlimit.vmem -= size;
			if (mm->extra_flags & EXTRA_FLAG_RLIMIT_DATA) {
				adp->rlimit.data -= size;
			}
			if(mm->extra_flags & EXTRA_FLAG_LOCK) {
				//RUSH1: BUG: This is no good - we need to call a function in 
				//RUSH1: vmm_munlock.c to actually unlock the range for the mm_map.
				adp->rlimit.memlock -= size;
			}
		}
		or = mm->obj_ref;
		if(or != NULL) {
			obp = or->obp;
			haslock = memobj_cond_lock(obp);

			//FUTURE: If we start turning on MM_MEM_HAS_SYNC
			//FUTURE: in vmm_vaddr_to_memobj(), we can check
			//FUTURE: it here and avoid the individual page checks

			//FUTURE: If the map region has never had write permissions,
			//FUTURE: we can skip this check, since we obviously can't
			//FUTURE: own a mutex: can't just check PROT_WRITE, since
			//FUTURE: we might have gotten the mutex and then turned off
			//FUTURE: PROT_WRITE with mprotect() - silly, but possible.
			//FUTURE: The loader threads will need to say that even though
			//FUTURE: they had PROT_WRITE on for a while, it doesn't count
			//FUTURE: against this check.
			//FUTURE: Since clean_pmem is now doing more than just dealing
			//FUTURE: with the sync object, have to be more careful about
			//FUTURE: skipping the call.

			data.mm = mm;
			data.flags = 0;
			(void)memobj_pmem_walk_mm(MPW_PHYS|MPW_SYSRAM, mm, clean_pmem, &data);

			if((data.flags & PAQ_FLAG_MODIFIED) 
			 && !(mm->mmap_flags & MAP_NOSYNCFILE)
			 && (obp->hdr.type == OBJECT_MEM_FD)
			 && (obp->mem.mm.flags & MM_FDMEM_NEEDSSYNC)) {
				//RUSH1: What to do if mm_sync() returns non-EOK?
				(void)mm_sync(obp, mm, MS_ASYNC);
			}

			//RUSH3: The mm_sync() call is also going to walk the
			//RUSH3: memory reference list. Can we combine the two?
			data.start = mm->offset;
			data.end   = mm->offset + (mm->end - mm->start);
			data.check_limit = 100;
			//RUSH3: Make more object oriented
			switch(obp->hdr.type) {
			case OBJECT_MEM_ANON:	
				if(obp->mem.mm.flags & MM_ANMEM_MULTI_REFS) {
					// We can't bail out early or we'll leak anon memory.
					data.check_limit = ~0;
				} else {
					// We can skip check_last_ref() if it's an anonymous object
					// with no mapping of "/proc/<pid>/as".
					data.check_limit = 0;
				}
				break;
			case OBJECT_MEM_SHARED:	
				if(obp->hdr.refs != 0) {
					// Can't free any memory yet.
					data.check_limit = 0;
					data.start = data.end + 1;
				}
				break;
			case OBJECT_MEM_FD:
				//RUSH3: If could tell if there were no open fd's,
				//RUSH3: we could start freeing memory here.
				data.check_limit = 0;
				data.start = data.end + 1;
				break;
			case OBJECT_MEM_TYPED:
				//RUSH3: Are there some conditions that will allow us
				//RUSH3: to skip the check? MAP_PRIVATE? non-allocable?
				// Can't bail out early or we'll leak memory
				data.check_limit = ~0;
				break;
			default: break;
			}
			if(data.check_limit != 0) {
				memref_walk(obp, check_last_ref, &data);
			}

			
#if defined(CPU_GBL_VADDR_START)
			//RUSH3: CPUism. ARM specific
			if(mm->extra_flags & EXTRA_FLAG_GBL_VADDR) {
				switch(obp->hdr.type) {
				case OBJECT_MEM_SHARED:
					// Tell CPU specific code this region is being unmapped
					cpu_gbl_unmap(adp, mm->start, mm->end, obp->mem.mm.flags);

					if (!(obp->mem.mm.flags & SHMCTL_GLOBAL)) {
						// Mapping is for this process only, so unmap it now
						// and release the global address range
						pte_unmap(adp, mm->start, mm->end, obp);
						(void)GBL_VADDR_UNMAP(mm->start, (mm->end - mm->start) + 1);
					} else if (data.start < data.end) {
						uintptr_t	va_start;
						uintptr_t	va_end;

						// Unmap only the range that is not referenced by
						// other processes
						// We can only release the global address range used
						// for the entire object when the object is destroyed.
						// The GBL_VADDR_UNMAP() is done in vmm_resize(). 
						va_start = mm->start + (uintptr_t)(data.start - mm->offset);
						va_end   = mm->start + (data.end - mm->offset);
						pte_unmap(adp, va_start, va_end, obp);
					}
					break;
				default: break;
				}
			} else
#endif
			{
				// If none of the quantums has the PAQ_FLAG_INITIALIZED
				// bit on (from clean_pmem), we know that we can't have set
				// up any page table entries, and can skip the unmapping
				if(data.flags & PAQ_FLAG_INITIALIZED) {
					pte_unmap(adp, mm->start, mm->end, obp);
				}
			}
			memref_del(mm);

			if(obp->hdr.type != OBJECT_MEM_ANON) {
				if(((mm->mmap_flags & (MAP_LAZY|MAP_TYPE)) == MAP_PRIVATE) 
					&& !(data.unmap_flags & UNMAP_PRIVATIZE)) {
					OBJECT		*anmem_obp;
					off64_t		anmem_off;
					size_t		anmem_size;
					int			anmem_lock;

					// For ~MAP_LAZY, MAP_PRIVATE, non-anonymous objects, we
					// allocated all the potential anon memory we'd need to
					// privatize the object in vmm_mmap() to avoid over-commitment
					// problems. We have to free that as well now.
					anmem_obp = adp->anon;
					anmem_size = (mm->end - mm->start) + 1;
					anmem_lock = memobj_cond_lock(anmem_obp);
					anmem_off = anmem_offset(anmem_obp, mm->start, anmem_size);
					memobj_pmem_del_len(anmem_obp, anmem_off, anmem_size);
					if(!anmem_lock) memobj_unlock(anmem_obp);
				}
			}
	
			if((data.flags & PAQ_FLAG_INUSE) && (data.start < data.end)) {
				memobj_pmem_del_len(obp, data.start, (size_t)(data.end - data.start) + 1);
			}

			if(!haslock) memobj_unlock(obp);
		}
	}
	if(has_unmapped) {
		map_remove(ms);
		map_destroy(ms);
	} else {
		map_coalese(ms);
	}

	return EOK;
}


int 
vmm_munmap(PROCESS *prp, uintptr_t vaddr, size_t len, int flags, part_id_t mpart_id) {
	int				r;
	struct map_set	ms;
	ADDRESS			*adp;

	if(prp == NULL) {
		CRASHCHECK(mpart_id == part_id_t_INVALID);

		//This code is assuming that all the memory we're unmapping
		//was pa_alloc'd - that is, a direct MAP_PHYS mapping never
		//gets unmapped. Right now the only time that kind of mapping is
		//done is for mounting an image file system and they never
		//get unmounted, so we should be safe for right now.
		if(CPU_1TO1_IS_VADDR(vaddr)) {
			CPU_1TO1_FLUSH(vaddr, len);
			pa_free_paddr(vaddr - CPU_1TO1_VADDR_BIAS, len, MEMPART_DECR(mpart_id, len));
			MEMCLASS_PID_FREE(prp, mempart_get_classid(mpart_id), len);
		} else {
#if CPU_SYSTEM_PADDR_MUST
			crash();
#else
			memsize_t memclass_pid_free = 0;
			va_rover = vaddr;
			do {
				paddr_t						paddr;

				(void) vmm_vaddrinfo(NULL, vaddr, &paddr, NULL, VI_PGTBL);
				pte_unmap(NULL, vaddr, vaddr + (QUANTUM_SIZE - 1), NULL);
				pa_free_paddr(paddr, QUANTUM_SIZE, MEMPART_DECR(mpart_id, QUANTUM_SIZE));
				memclass_pid_free += QUANTUM_SIZE;
				vaddr += QUANTUM_SIZE;
				len -= QUANTUM_SIZE;
			} while(len != 0);
			MEMCLASS_PID_FREE(prp, mempart_get_classid(mpart_id), memclass_pid_free);
#endif			
		}
		return EOK;
	}

	adp = prp->memory;
	r = map_isolate(&ms, &adp->map, vaddr, len, MI_SPLIT);
	if((r == EOK) && (ms.first != NULL)) {
		r = ms_unmap(adp, &ms, flags);
	}
	return r;
} 

