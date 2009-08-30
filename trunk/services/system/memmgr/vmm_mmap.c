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

//
//BACKWARDS
//Sigh. Old memory manager let people access beyond the end of
//the object if it was MAP_PRIVATE|MAP_ELF (for BSS). I'd change 
//elf_load() to do things properly, but the same code is in the
//shared object loader in libc, and I want to allow old libc's to
//continue working. 
//

//RUSH2: Use _mmap2() in elf_load() and ldd() in libc so this code
//RUSH2: knows both p_filesz & p_memsz values? 

//RUSH3: Check for and remove extra msync/mprotect's in ldd().

//RUSH1: Switch ldd() in libc to properly mmap()
//RUSH1: anon memory for the BSS section and then remove this hack
//RUSH1: after a couple of releases?
#define LOADER_BSS_KLUDGE


uintptr_t	(*vaddr_search_adjust_hook)(uintptr_t vaddr, 
					ADDRESS *adp, unsigned flags, uintptr_t size);
void (*pmem_stats_hook)(OBJECT *obp, size_t size, unsigned type);

			
#if defined(CPU_GBL_VADDR_START)
static struct mm_map_head	gbl_map;
static pthread_mutex_t		gbl_mux = PTHREAD_MUTEX_INITIALIZER;

void
gbl_vaddr_init(void) {
	if(map_init(&gbl_map, CPU_GBL_VADDR_START, CPU_GBL_VADDR_END) != EOK) {
		crash();
	}
}

int
gbl_vaddr_unmap(uintptr_t vaddr, size_t len) {
	struct map_set	ms;
	int				r;

	pthread_mutex_lock(&gbl_mux);
	r = map_isolate(&ms, &gbl_map, vaddr, len, MI_SPLIT);
	if(r == EOK) {
		if(ms.first != NULL) {
			map_remove(&ms);
			map_destroy(&ms);
		}
	}
	pthread_mutex_unlock(&gbl_mux);
	return r;
}
#endif


static int
tymem_valid_offset(off64_t off, size_t size, struct pa_restrict *list) {
	struct pa_restrict	*rp;
	int					found;
	off64_t				end = off + size - 1;
	
	if(off < end) {
		do {
			found = 0;
			for(rp = list; rp != NULL; rp = rp->next) {
				if((off >= rp->start) && (off <= rp->end)) {
					off = rp->end + 1;
					found = 1;
				}
				if((end >= rp->start) && (end <= rp->end)) {
					end = rp->start - 1;
					found = 1;
				}
				if(end < off) return EOK;
			}
		} while(found);
	}
	return ENXIO;
}


struct tymem_pmem {
	off64_t				off;
	unsigned			end_quanta;
	int					covering;
	struct pa_quantum	*pq;
	struct pa_quantum	**owner;
};

static int
tymem_fakes(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct tymem_pmem	*data = d;
	struct pa_quantum	*next;

	data->covering = 1;
	pq = pa_alloc_fake(off, NQUANTUM_TO_LEN(num));
	if(pq == NULL) {
		for(pq = data->pq; pq != NULL; pq = next) {
			next = pq->u.inuse.next;
			pa_free_fake(pq);
		}
		return ENOMEM;
	}
	*data->owner = pq;
	data->owner = &pq->u.inuse.next;
	return EOK;
}

static int
tymem_split(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct tymem_pmem	*data = d;
	unsigned			end_quanta;

	end_quanta = LEN_TO_NQUANTUM(off) + num;
	if((data->off <= off) || (data->end_quanta >= end_quanta)) {
		return EOK;
	}
	// need to split
	return memobj_pmem_split(obp, pq, LEN_TO_NQUANTUM(data->off - off));
}

int
tymem_pmem_alloc(PROCESS *prp, OBJECT *obp, struct map_set *ms, struct mm_map *mm, 
			unsigned flags, off64_t boff, size_t size) {
	int					r;
	struct pa_quantum	*pq_add;
	struct pa_quantum	*pq_next;
	struct pa_quantum	*pq;
	struct mm_map		*mm_add;
	unsigned			pg_offset;
	struct tymem_pmem	data;
	unsigned			pa_flags;
	size_t				split_point;
	size_t				run_len;

	if(flags & (MAP_LAZY|MAP_BELOW16M)) return EINVAL;
	pg_offset = ADDR_OFFSET(boff);
	boff -= pg_offset;
	size += pg_offset;
	if(flags & (IMAP_TYMEM_ALLOCATE|IMAP_TYMEM_ALLOCATE_CONTIG)) {
		if((boff != 0) || (pg_offset != 0)) return EINVAL;
		pa_flags = 0;
		if(flags & IMAP_TYMEM_ALLOCATE_CONTIG) pa_flags = PAA_FLAG_CONTIG;
		//RUSH2: Alignment, colour....
		if(obp->mem.mm.flags & MM_MEM_RDB) {
			pq = tymem_rdb_alloc(obp, size, pa_flags);
		} else {
			memsize_t  resv = 0;
			if (MEMPART_CHK_and_INCR(obp->hdr.mpid, size, &resv) != EOK) {
				return ENOMEM;
			}
			pq = pa_alloc(size, 0, PAQ_COLOUR_NONE, pa_flags, NULL,
						obp->mem.mm.restriction, resv);
			if (pq == NULL) {
				MEMPART_UNDO_INCR(obp->hdr.mpid, size, resv);
			} else {
				MEMCLASS_PID_USE(prp, mempart_get_classid(obp->hdr.mpid), size);
			}
		}
		if(pq == NULL) return ENOMEM;
	} else if(flags & IMAP_TYMEM_MAP_ALLOCATABLE) {
		r = tymem_valid_offset(boff, size, obp->mem.mm.restriction);
		if(r != EOK) return r;
		data.pq = NULL;
		data.owner = &data.pq;
		// Create fake quantums for all the regions that don't already have
		// pmem entries.
		r = memobj_pmem_walk(MPW_HOLES, obp, boff, boff + size - 1, 
								tymem_fakes, &data);
		if(r != EOK) return r;
		pq = data.pq;
		mm->obj_ref = (void *)pq;
		mm->offset = boff;
	} else {
		r = tymem_valid_offset(boff, size, obp->mem.mm.restriction);
		if(r != EOK) return r;
		if(obp->mem.mm.flags & MM_MEM_RDB) {
			int		tmp;

			pq = tymem_rdb_alloc_given(obp, boff, size, &tmp);
			r = tmp;
		} else {
			pq = pa_paddr_to_quantum(boff);
			if(pq == NULL) return ENXIO;
			r = pa_alloc_given(pq, LEN_TO_NQUANTUM(size), NULL);
		}
		if(r != EOK) return r;
	}
	if((pq->blk != PAQ_BLK_FAKE) || (pq->flags & PAQ_FLAG_RDB)) {

		// We're hiding the quantum pointer for the mapping in the 'obj_ref'
		// field in this section. It'll all be cleaned up before we leave. 

		split_point = mm->start - ms->first->start;
		mm_add = mm;
		for(pq_add = pq; pq_add != NULL; pq_add = pq_next) {
			pq_next = pq_add->u.inuse.next;
			mm_add->obj_ref = (void *)pq_add;
			mm_add->offset = pa_quantum_to_paddr(pq_add);
			data.off = mm_add->offset;
			data.end_quanta = LEN_TO_NQUANTUM(data.off) + pq_add->run;
			run_len = NQUANTUM_TO_LEN(pq_add->run);
			if((data.off + run_len) > obp->mem.mm.size) {
				obp->mem.mm.size = data.off + run_len;
			}
			data.covering = 0;
			// Since sysram quantums override fake ones, we're going to
			// delete all the regions covered by fake entries when we
			// get around to adding the memory to the pmem list down
			// below. Now, we might have to allocate memory to delete
			// from the middle of a fake entry, which means that
			// we might run out in the middle of adding, which would
			// be really obnoxious to back out of. Instead, we'll run
			// through and split the problematic fake entries up ahead
			// of time. That way, if we're going to run out of memory, 
			// we'll run out here where it's easier to undo things.
			r = memobj_pmem_walk(MPW_PHYS, obp, mm->offset, 
							mm->offset + run_len - 1, tymem_split, &data);
			if(r != EOK) goto fail1;
			// Keep track of the fact that we need to clean the region
			// before adding the new pmem entries.
			mm_add->reloc = data.covering;
			if(pq_next != NULL) {
				split_point += run_len;
				r = map_split(ms, split_point);
				if(r != EOK) goto fail1;
				mm_add = mm_add->next;
			}
			pq_add->u.inuse.next = NULL;
		}
	}
	for( ;; ) {
		pq_add = (void *)mm->obj_ref;
		mm->obj_ref = NULL;
		while(pq_add != NULL) {
			off64_t		add_off;

			pq_next = pq_add->u.inuse.next;
			add_off = pa_quantum_to_paddr(pq_add);
			if(mm->reloc) {
				mm->reloc = 0;
				memobj_pmem_del_len(obp, add_off, NQUANTUM_TO_LEN(pq_add->run));
			}
			(void) memobj_pmem_add_pq(obp, add_off, pq_add);
			pq_add = pq_next;
		}
		if(mm == ms->last) break;
		mm = mm->next;
	} 
	//RUSH3: It'd be nice to collapse adjacent, contiguous fake quantums
	return EOK;

fail1:
	for( ;; ) {
		pq_add = (void *)mm->obj_ref;
		mm->obj_ref = NULL;
		while(pq_add != NULL) {
			pq_next = pq_add->u.inuse.next;
			if(pq_add->blk == PAQ_BLK_FAKE) {
				if(pq_add->flags & PAQ_FLAG_RDB) {
					tymem_rdb_free(obp, pa_quantum_to_paddr(pq_add), NQUANTUM_TO_LEN(pq_add->run));
				}
				pa_free_fake(pq_add);
			} else {
				size_t  len = NQUANTUM_TO_LEN(pq_add->run);
				pa_free(pq_add, pq_add->run, MEMPART_DECR(obp->hdr.mpid, len));
				MEMCLASS_PID_FREE(prp, mempart_get_classid(obp->hdr.mpid), len);
			}
			pq_add = pq_next;
		}
		if(mm == ms->last) break;
		mm = mm->next;
	} 
	return r;
}


static int rdecl
setup_mappings(PROCESS *prp, struct map_set *ms, unsigned flags, unsigned preload) {
	ADDRESS					*adp;
	size_t					maxsize;
	size_t					todo;
	uintptr_t				start;
	uintptr_t				end;
	int						r;
	struct mm_object_ref	*or;
	OBJECT					*obp;
	unsigned				mr_flags;
	struct mm_map			*mm;

	preload = ROUNDUP(preload, __PAGESIZE);
	r = EOK;
	adp = prp->memory;
	if(flags & MAP_STACK) {
#ifdef STACK_GROWS_UP
		mm = ms->first;
		start = mm->start;
		end = start + preload - 1;
		if(end > mm->end) end = mm->end;
#else
		mm = ms->last;
		end = mm->end;
		start = ( end - preload ) + 1;
		if(start < mm->start) start = mm->start;
#endif
		todo = ( end - start ) + 1;
		if(mm->extra_flags & EXTRA_FLAG_PRIMARY_STK) {
			if(rlimit_blown(prp, RLIMIT_STACK, todo)) {
				return ENOMEM;
			}
			prp->memory->rlimit.stack = todo;
		}
		r = memory_reference(&mm, start, end, 
				(mm->mmap_flags & PROT_WRITE) ? (MR_WRITE|MR_TRUNC) : MR_TRUNC, ms);
	} else {
		// memory_reference() might be fiddling the list
		uintptr_t	end_vaddr = ms->last->end;

		mr_flags = (flags & PROT_WRITE) ? (MR_WRITE|MR_TRUNC) : MR_TRUNC;
		mm = ms->first;
		for( ;; ) {
			or = mm->obj_ref;
			if(or != NULL) {
				obp = or->obp;
				start = mm->start;
				end   = mm->end;
				maxsize = ( end - start ) + 1;
				todo = preload;
				if(todo > maxsize) todo = maxsize;
				if(!(flags & MAP_SYSRAM)) {
					// Doing a direct physical mapping - just set up the
					// page tables right away.
					r = memory_reference(&mm, start, end, MR_NOINIT, ms);
				} else if(!(flags & MAP_LAZY) && !SHM_LAZY(obp)) {
					uintptr_t	s;
					uintptr_t	e;

					//FUTURE: On systems with referenced/modified bits in the
					//FUTURE: PTE's (e.g. X86), if we have the pmem allocated
					//FUTURE: and initialized, we can set up the page tables
					//FUTURE: here and avoid the initial fault for refer/modifying.
					//FUTURE: In that case though, we'd have to or together the
					//FUTURE: PTE bits from all the pages tables that reference
					//FUTURE: the pmem.
					//RUSH3: If !OBJECT_MEM_FD, start zeroing in the background?
					if(todo > 0) {
						r = memory_reference(&mm, start, start + (todo-1), mr_flags, ms);
						if(r != EOK) return r;
					}
					// Careful - memory_reference() might adjust 'mm'
					s = start + todo;
					e = mm->end;

					if(s < e) {
						// Have to make sure all the L2's are allocated
						r = pte_prealloc(adp, s, e);
						if((r == EOK) && (mm->obj_ref->obp == obp)) {
							// If we happen to have init'd pages, 
							// set up the PTE's.
							r = memory_reference(&mm, s, e, MR_NOINIT, ms);
						}
					}
				} else if(todo > 0) {
					r = memory_reference(&mm, start, start+todo-1, mr_flags, ms);
				}
				if(r != EOK) return r;
				preload -= todo;
#if defined(CPU_GBL_VADDR_START)
				//RUSH3: CPUism. ARM specific
				if(mm->extra_flags & EXTRA_FLAG_GBL_VADDR) {
					switch(obp->hdr.type) {
					case OBJECT_MEM_SHARED:
					case OBJECT_MEM_TYPED:
						// Tell CPU specific code about the mapping
						(void) cpu_gbl_mmap(adp, mm->start, mm->end, obp->mem.mm.flags);
						break;
					default: break;
					}
				}
#endif
			}
			if(mm->end >= end_vaddr) break;
			mm = mm->next;
		}
	}
	return r;
}


int 
vmm_mmap(PROCESS *prp, uintptr_t vaddr_requested, size_t size_requested, 
		int prot, int flags, OBJECT *obp, uint64_t boff, unsigned alignval, 
		unsigned preload, int fd, void **vaddrp, size_t *sizep, part_id_t mpart_id) {
	uintptr_t			vaddr;
	size_t				size;
	size_t				guardsize = 0;
	size_t				pg_offset;
	ADDRESS				*adp;		
	struct mm_map_head	*mh;
	struct map_set		ms;
	struct map_set		ms_repl;
	int					r;
	struct mm_map		*mm;
	struct pa_quantum	*pq = NULL;
	uintptr_t			mask;
	unsigned			extra_flags=0;
	unsigned			create_flags;
	unsigned 			obp_flags;
	uintptr_t			global_vaddr;
	OBJECT				*anmem_obp;
	off64_t				anmem_off;
#ifdef LOADER_BSS_KLUDGE
	void				*kludge_vaddr;
	size_t				kludge_size;
	part_id_t		kludge_mpart_id;
#endif	

	//RUSH2: /proc/<pid>/as support. Remember to turn on the MM_ANMEM_MULTI_REFS
	//RUSH2: bit for anonymous objects so we don't skip check_last_ref
	//RUSH2: in vmm_munmap.
	//RUSH2: What should I do with 'alignval'?

	flags |= prot;
	if(flags & MAP_STACK) {
		if(!(flags & MAP_ANON) || ((size_t)boff >= size_requested)) {
			return EINVAL;
		}
		// calculate guardpage size (in boff)
		guardsize = ROUNDUP((size_t)boff, __PAGESIZE);
		boff = 0;
	}

	if(prp == NULL) {
		// mmap system memory...

		// If the MAP_SYSRAM bit is on, this is a special hook from
		// the loader_elf.c file telling us where the procnto code
		// and data are. Yuck.
		if(flags & MAP_SYSRAM) {
			struct vaddr_range	*seg;

			if(prot & PROT_WRITE) {
				seg = &system_code_vaddr;
			} else {
				seg = &system_data_vaddr;
			}
			seg->lo = vaddr_requested;
			seg->hi = vaddr_requested + size_requested - 1;
			return EOK;
		}

		if((obp != NULL)
			&& (obp->hdr.type == OBJECT_MEM_SHARED)
			&& (obp->mem.mm.flags & SHMCTL_PHYS)
			&& !(obp->mem.mm.flags & SHMCTL_ANON)) {
			// Direct mapping
			boff += pa_quantum_to_paddr(obp->mem.mm.pmem);
			flags &= ~MAP_ANON;
			flags |= MAP_PHYS;
			obp = NULL;
		}

		CRASHCHECK(flags & PROT_NOCACHE);
		CRASHCHECK(obp != NULL);
#ifndef NDEBUG
		/*
		 * only during initial startup (ie once) will mpart_id be
		 * part_id_t_INVALID and sys_mempart and procnto_prp both
		 * be NULL. We allow this to go through otherwise we assert
		 * mpart_id != part_id_t_INVALID
		*/
		if (mpart_id == part_id_t_INVALID) {
			extern void *sys_mempart;
			if (!((sys_mempart == NULL) && (procnto_prp == NULL))) crash();
		}
#endif	/* NDEBUG */

		if(flags & MAP_STACK) size_requested -= guardsize;
		if(flags & MAP_ANON) {
			//
			// We're always asking for contiguous memory, but that might not 
			// always be required. It should be OK though, since the vast 
			// majority of system allocations are only going to be one page.
			//
			memsize_t  resv = 0;

			size_requested = ROUNDUP(size_requested, QUANTUM_SIZE);
			if (MEMPART_CHK_and_INCR(mpart_id, size_requested, &resv) != EOK) {
				return ENOMEM;
			}
			pq = pa_alloc(size_requested, 0, PAQ_COLOUR_NONE, 
							PAA_FLAG_CONTIG, NULL, restrict_proc, resv);
			if(pq == NULL) {
				MEMPART_UNDO_INCR(mpart_id, size_requested, resv);
				return ENOMEM;
			}
			/* prp is NULL, so this call is a no-op anyway. Can't leave it in
			 * because mempart_get_classid(mpart_id) will fault because 'mpart_id'
			 * is part_id_t_INVALID on startup as per NDEBUG check above (this was
			 * seen with a 4.2.1 gcc build)
			MEMCLASS_PID_USE(prp, mempart_get_classid(mpart_id), size_requested);
			*/
			pq->flags |= PAQ_FLAG_SYSTEM;
			boff = pa_quantum_to_paddr(pq);
			pg_offset = 0;
		} else if(flags & MAP_PHYS) {
			// Have to validate the paddr (passed in in 'boff') because 
			// mounting an image file system gets given the paddr from
			// user input and the code in memmmgr_map.c that normally
			// checks for a good value is being bypassed.
			if((boff > last_paddr) || ((boff+size_requested-1)) > last_paddr) {
				return EINVAL;
			}
			pg_offset = boff - ROUNDDOWN(boff, __PAGESIZE);
			boff -= pg_offset;	
			size_requested = ROUNDUP(size_requested+pg_offset, QUANTUM_SIZE);
		} else {
			crash();
			/* NOTREACHED */
			pg_offset = 0;
		}
		*sizep = size_requested;
		if(CPU_1TO1_IS_PADDR(boff)) {
			vaddr = (uintptr_t)boff + CPU_1TO1_VADDR_BIAS;
		} else {
#if CPU_SYSTEM_PADDR_MUST
			if(!(flags & MAP_ANON)) {
				// image file systems mount - can't do it in the sysaddr
				// space if we're outside the 1-to-1 region.
				return ENOTSUP;
			}
			crash();
			/* NOTREACHED */
			vaddr = 0;
#else
			struct mm_pte_manipulate	data;

			data.end = size_requested;
			data.prot = flags;
			data.paddr = boff;
			if(KerextAmInKernel()) {
				sysaddr_map(&data);
			} else {
				__Ring0(sysaddr_map, &data);
			}
			vaddr = data.start;
			if(vaddr == VA_INVALID) {
				if(flags & MAP_ANON) {
					pa_free_list(pq, MEMPART_DECR(mpart_id, size_requested));
					MEMCLASS_PID_FREE(prp, mempart_get_classid(mpart_id), size_requested);
				}
				return ENOMEM;
			}
#endif
		}
#if CPU_SYSTEM_HAVE_COLOURS
		if(pq != NULL) {
			colour_set(vaddr, pq, pq->run);
		}
#endif
		*vaddrp = (uint8_t *)vaddr + pg_offset;
		if(pq != NULL) {
			if(vaddr < system_heap_vaddr.lo) system_heap_vaddr.lo = vaddr;
			vaddr += size_requested - 1;
			if(vaddr > system_heap_vaddr.hi) system_heap_vaddr.hi = vaddr;
		}
		return EOK;
	}
	
	// mmap user memory...
	adp = prp->memory;

	// make sure our heap hasn't exceeded its rlimit
	if((flags & (MAP_ANON|MAP_STACK|MAP_ELF|MAP_TYPE)) == (MAP_ANON|MAP_PRIVATE)) {
		if(rlimit_blown(prp, RLIMIT_DATA, adp->rlimit.data + size_requested)) {	
			return ENOMEM;
		}
		extra_flags |= EXTRA_FLAG_RLIMIT_DATA;
	}

	// make sure our total vm size hasn't exceeded its rlimit
	if(rlimit_blown(prp, RLIMIT_VMEM, adp->rlimit.vmem + size_requested)) {
		return ENOMEM;
	}

#ifdef LOADER_BSS_KLUDGE	
	kludge_vaddr = NULL;
	kludge_size = 0;
	kludge_mpart_id = NULL;	// for the compiler
	if((obp != NULL) 
		&& ((flags & (MAP_TYPE|MAP_ANON|MAP_ELF|MAP_FIXED)) == (MAP_PRIVATE|MAP_ELF|MAP_FIXED))
		&& !(prp->flags & _NTO_PF_LOADING)) {
		uintptr_t	end_vaddr;

		switch(obp->hdr.type) {
		case OBJECT_MEM_FD:
		case OBJECT_MEM_SHARED:
			end_vaddr = ROUNDUP(vaddr_requested + size_requested, __PAGESIZE);
			vaddr = ROUNDUP(vaddr_requested 
							+ (unsigned)(obp->mem.mm.size - boff), __PAGESIZE);
			if(vaddr < end_vaddr) {
				kludge_mpart_id = mempart_getid(prp, sys_memclass_id);
				r = vmm_mmap(prp, vaddr, end_vaddr - vaddr, 
							prot & PROT_MASK, MAP_PRIVATE|MAP_ANON|MAP_FIXED, 
							NULL, 0, 0, 0, NOFD, &kludge_vaddr, &kludge_size, kludge_mpart_id);
				if(r != EOK) return r;
				size_requested = vaddr - vaddr_requested;
			}
			break;
		default: break;
		}
	} 
#endif	

	obp_flags = 0;
	if(prp->flags & _NTO_PF_LOADING) extra_flags |= EXTRA_FLAG_LOADER;
	if(flags & IMAP_OCB_RDONLY) {
		if((flags & PROT_WRITE) && !(extra_flags & EXTRA_FLAG_LOADER)) {
			// Report an error if we try to gain PROT_WRITE perms to
			// an object that's opened for read-only, unless it's the
			// loader doing it - that happens for use-in-place data segments
			r = EACCES;
			goto fail0;
		}
		extra_flags |= EXTRA_FLAG_RDONLY;
	}
		
	global_vaddr = VA_INVALID;
	//RUSH3: CPUism
	//RUSH3: This code is only correct for the ARM right now. Need better API
#if defined(CPU_GBL_VADDR_START)
	if((obp != NULL) 
	 && ((obp->hdr.type == OBJECT_MEM_SHARED)||(obp->hdr.type == OBJECT_MEM_TYPED))
	 && !(obp->mem.mm.flags & MM_SHMEM_IFS)
	 //RUSH3: Do we really have to check for MM_SHMEM_SPECIAL?
	 //RUSH3: Wouldn't just looking at the SHMCTL_* bits be sufficient?
	 && (obp->mem.mm.flags & MM_SHMEM_SPECIAL)
	 && ((obp->mem.mm.flags & SHMCTL_GLOBAL) ||
	     (obp->mem.mm.flags & (SHMCTL_ANON|SHMCTL_PHYS)) == SHMCTL_PHYS)
	 && ((flags & MAP_TYPE) == MAP_SHARED)) {
		//RUSH3: ARMism (make conditional via minherit()?)
		extra_flags	|= EXTRA_FLAG_NOINHERIT;
		global_vaddr = obp->shmem.vaddr;
		if(global_vaddr == VA_INVALID) {
			pthread_mutex_lock(&gbl_mux);
			//RUSH3: ARMism
			if(obp->mem.mm.flags & SHMCTL_GLOBAL) {
				if(obp->mem.mm.size == 0) return EINVAL;
				//RUSH3: ARMism (1 megabyte vaddr alignment)
				r = map_create(&ms, &ms_repl, &gbl_map, 0, 
						ROUNDUP(obp->mem.mm.size, __PAGESIZE), 
						ADDR_PAGE((1024*1024)-1), 0);
			} else {
				//RUSH3: ARMism (1 megabyte vaddr alignment)
				r = map_create(&ms, &ms_repl, &gbl_map, 0, 
						ROUNDUP(size_requested, __PAGESIZE), 
						ADDR_PAGE((1024*1024)-1), 0);
			}
			if(r != EOK) {
				pthread_mutex_unlock(&gbl_mux);
				goto fail0;
			}
			global_vaddr = ms.first->start;
			(void) map_add(&ms);
			map_coalese(&ms);
			pthread_mutex_unlock(&gbl_mux);
		}
		if(obp->mem.mm.flags & SHMCTL_GLOBAL) {
			obp->shmem.vaddr = global_vaddr;
			// We don't have to unmap the vaddr from the global
			// map because it's been permanently assigned to the object
			if(boff > obp->mem.mm.size) return EINVAL;
			global_vaddr += boff;
		}
		extra_flags |= EXTRA_FLAG_GBL_VADDR;
	}
#endif

	pg_offset = ADDR_OFFSET((size_t)boff);
	if(flags & MAP_FIXED) {
		unsigned	addr_offset;

		r = EINVAL; // Assuming a failure in this section
		if((global_vaddr != VA_INVALID) && (global_vaddr != vaddr_requested)) {
			goto fail1;
		}
		// Make sure addr and offset are at the same offset modulo pagesize
		addr_offset = ADDR_OFFSET(vaddr_requested);
		if(pg_offset != addr_offset) {
			pg_offset = addr_offset;
			if(obp == NULL) {
				// physical mappings have to have addr/offset congruency
				if((flags & MAP_PHYS) && !(flags & MAP_ANON)) goto fail1;
			} else if(obp->hdr.type == OBJECT_MEM_TYPED) {
				// non-allocated typed memory have to have addr/offset congruency
				if(!(flags & (IMAP_TYMEM_ALLOCATE|IMAP_TYMEM_ALLOCATE_CONTIG))) goto fail1;
			} else {
				// otherwise addr & offset have to be congruent
				goto fail1;
			}

		}
		// Don't allow a guardsize with MAP_FIXED
		guardsize = 0;
		vaddr = vaddr_requested;
	} else if(global_vaddr != VA_INVALID) {
		vaddr = global_vaddr;
	} else {
		if(flags & MAP_STACK) {
			vaddr = prp->base_addr;
		} else if(vaddr_requested != 0) {
			vaddr = vaddr_requested;
			if(!(flags & MAP_BELOW)) {
				vaddr = ROUNDUP(vaddr, __PAGESIZE);
			}
		} else if(flags & MAP_ELF) {
			vaddr = CPU_SO_VADDR_START;
		} else if(flags & MAP_ANON) {
			vaddr = prp->base_addr;
		} else {
			vaddr = CPU_SHMEM_VADDR_START;
		}
		if(vaddr_search_adjust_hook != 0) {
			vaddr = vaddr_search_adjust_hook(vaddr, adp, flags, size_requested);
		}
	}
	boff -= pg_offset;
	size = ROUNDUP(size_requested + pg_offset, __PAGESIZE);
	if(size < size_requested) {
		r = ENOMEM;
		goto fail1;
	}

	mh = &adp->map;

	mask = 0;
#if CPU_SYSTEM_HAVE_COLOURS
	if(obp != NULL) {
		unsigned	colour;

		colour = memobj_colour(obp, boff);
		if(colour != PAQ_COLOUR_NONE) {
			mask = colour_mask_shifted;
			if(flags & MAP_BELOW) {
				vaddr = vaddr - (((vaddr & mask) - (colour << ADDR_OFFSET_BITS)) & mask);
			} else {
				vaddr = vaddr + (((colour << ADDR_OFFSET_BITS) - (vaddr & mask)) & mask);
			}
			if((flags & MAP_FIXED) && (vaddr != vaddr_requested)) {
				if(!(flags & MAP_ELF)) {
					r = EINVAL;
					goto fail1;
				}
				// We've got a colour mismatch between the object
				// and the requested vaddr, but it's OK because
				// it's for an executable (e.g. running from "/dev/shmem").
				obp_flags |= MM_MEM_COLOUR_FLUSH;
				vaddr = vaddr_requested;
			}
		}
	}
#endif	

	anmem_obp = adp->anon;
	//RUSH2: Need to deal with alignment
	//RUSH2: Need to deal with /proc/<pid>/as objects...
	create_flags = flags;
	if(extra_flags & EXTRA_FLAG_GBL_VADDR) create_flags |= IMAP_GLOBAL;
#ifndef STACK_GROWS_UP
	if(create_flags & MAP_STACK) create_flags |= MAP_BELOW;
#endif
	if((mask == 0) && (vaddr_requested == 0) && (mm_flags & MM_FLAG_VPS)) {
		// If we don't have an imposed alignment and no restrictions
		// on the vaddr we allocate, tell map_create() that it should
		// try to make (but not require) the allocated vaddr to be 
		// aligned in such a manner that we can use big pages, but first
		// check that the mapping could possibly create a big page.
		switch(CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES) {
		case VPS_HIGHUSAGE:	
			// Only shared memory objects with SHMCTL_HIGHUSAGE on will
			// potentially use big pages (e.g. PPC 600 family, Freescale E500)
			if((obp == NULL) 
			  || (obp->hdr.type != OBJECT_MEM_SHARED)
			  || !(obp->mem.mm.flags & SHMCTL_HIGHUSAGE)) break;
			// Fall through
		case VPS_AUTO:	
			// All mappings could potentially use big pages.
			mask = 1;
			break;
		default:
			// VPS_NONE: No mappings will ever use a big page.
			break;
		}
	}
	r = map_create(&ms, &ms_repl, mh, ADDR_PAGE(vaddr), size, mask, create_flags);
	if(r != EOK) goto fail1;
	mm = ms.first;
	if((flags & MAP_STACK) && (extra_flags & EXTRA_FLAG_LOADER)) {
		// If it's a MAP_STACK while we're loading the process, this
		// must be the primary stack - remember that for RLIMIT_STACK
		// handling.
		extra_flags |= EXTRA_FLAG_PRIMARY_STK;
	}

	if((flags & (MAP_TYPE|MAP_ELF|MAP_FIXED)) == (MAP_ELF|MAP_FIXED|MAP_PRIVATE)) {
		uintptr_t	end;

		// For MAP_ELF, the size_requested is the phdr->p_filesz field,
		// so we really can't read beyond that point even if the file
		// object allows it - the data may not be all zeros. Instead
		// we remember how much of the last page is 'missing' and code
		// in mm_reference.c will zero it when the page gets initialized.

		//RUSH3: This is assuming that only the MAP_PRIVATE mapping
		//RUSH3: has phdr->p_filesz < phdr->p_memsz and that there's
		//RUSH3: no other segment data following it. Think about reading
		//RUSH3: in the segment table in memmgr_fd.c so that we can consult
		//RUSH3: it in mm_reference.c (and we wouldn't need this code anymore,
		//RUSH3: just a bit in extra_flags that pq_init() needed to do some
		//RUSH3: extra checks).
		end = vaddr_requested + size_requested;
		mm->last_page_bss = ROUNDUP(end, __PAGESIZE) - end;
	}

	vaddr = mm->start;
	if(guardsize != 0) {
		r = map_split(&ms, guardsize);
		if(r != EOK) goto fail2;
		size -= guardsize;
		mm = ms.first;
#ifdef STACK_GROWS_UP
		// With an upwardly growing stack, the guard page is at end of list	
		mm->next->mmap_flags = MAP_STACK;
#else		
		mm->mmap_flags = MAP_STACK;
		mm = mm->next;
#endif		
	}

	//RUSH3: We can get more sophisticated and only zero the temp mapping
	//RUSH3: area size if the [vaddr, mm->end) range is overlapping with
	//RUSH3: it.
	adp->tmap_size = 0;

	anmem_off = ~(off64_t)0;
	if(obp == NULL) {
		obp = anmem_obp;
		if(ANMEM_MULTI_REFS(obp)) {
			memobj_lock(obp);
		}
	}

	if(!(flags & MAP_ANON)) {
		r = memobj_offset_check(obp, boff, size);
		if(r != EOK) goto fail2;
	}

	if(ms_repl.first != NULL) {
		r = ms_unmap(adp, &ms_repl, 0);
		if(r != EOK) goto fail2;
	}

	if(obp->hdr.type == OBJECT_MEM_TYPED) {
		r = tymem_pmem_alloc(prp, obp, &ms, mm, flags, boff, size);
		if(r != EOK) goto fail3;
		if(flags & (IMAP_TYMEM_ALLOCATE|IMAP_TYMEM_ALLOCATE_CONTIG)) {
			if(!(obp->mem.mm.flags & MM_MEM_RDB)) {
				flags |= MAP_SYSRAM;
			}
		}
	} else if(obp->hdr.type == OBJECT_MEM_FD) {
		//RUSH3: For MAP_PRIVATE, we're going to end up allocating
		//RUSH3: more pmem when we privatize(). Maybe we should just
		//RUSH3: get the anon memory up front and read the file data
		//RUSH3: into it here. Doing it the current way is faster for for 
		//RUSH3: multiple mappings of the file, since we'll have a cached copy
		//RUSH3: in RAM of the data for the second time. Maybe allow
		//RUSH3: the user to choose via some flag to mmap()?
		if(!(flags & MAP_LAZY)) {
			r = memobj_pmem_add(obp, boff, size, flags);
			if(r != EOK) goto fail3;
		}
		flags |= MAP_SYSRAM;
	} else if(flags & MAP_ANON) {
		boff = anmem_offset(obp, mm->start, size);
		if(!(flags & MAP_LAZY)) {
			r = memobj_pmem_add(obp, boff, size, flags);
			if(r != EOK) goto fail3;
		}
		flags |= MAP_SYSRAM;
	} else if(flags & MAP_PHYS) {
		off64_t	noff;

		noff = anmem_offset(obp, mm->start, size);
		pq = pa_alloc_fake(ADDR_PAGE(boff), size);
		if(pq == NULL) goto fail3;
		r = memobj_pmem_add_pq(obp, noff, pq);
		if(r != EOK) goto fail4;
		boff = noff;
		flags &= ~MAP_LAZY;
	} else if(obp->hdr.type == OBJECT_MEM_SHARED) {
		if((obp->mem.mm.flags & (MM_SHMEM_SPECIAL|SHMCTL_ANON|SHMCTL_PHYS)) != (MM_SHMEM_SPECIAL|SHMCTL_PHYS)) {
			flags |= MAP_SYSRAM;
		}
	}
	if(((flags & (MAP_LAZY|MAP_TYPE)) == MAP_PRIVATE) && (obp->hdr.type != OBJECT_MEM_ANON)) {
		// For ~MAP_LAZY, MAP_PRIVATE, non anonymous memory, pre-allocate
		// the all anonymous memory that we might end up needing.
		// This will avoid overcommiting the memory and having processes
		// randomly dying with SIGBUS's when we find that we need to create
		// a private copy of a page and we're out of physical memory.
		memobj_lock(anmem_obp);
		anmem_off = anmem_offset(anmem_obp, mm->start, size);
		r = memobj_pmem_add(anmem_obp, anmem_off, size, flags);
		memobj_unlock(anmem_obp);
		if(r != EOK) goto fail4;
	}

	r = map_add(&ms);
	if(r != EOK) goto fail5;
	for( ;; ) {
		// Typed memory offsets already set by tymem_pmem_alloc()...
		if(obp->hdr.type != OBJECT_MEM_TYPED) {
			mm->offset = boff;
		}
		// The IMAP_MASK includes MAP_SYSRAM, which we want to allow through here
		mm->mmap_flags = flags & (~IMAP_MASK | MAP_SYSRAM);
		mm->extra_flags |= extra_flags;
		r = memref_add(mm, obp, adp, fd);
		if(r != EOK) goto fail6;
		if(mm == ms.last) break;
		boff += ( mm->end - mm->start ) + 1;
		mm = mm->next;
	}
	
	obp->mem.mm.flags |= obp_flags;

	if(preload == 0) {
		//
		// We have some heuristics to optimize performance. We detect 
		// a few different cases and try to pre-fault in some of the pages.
		//
		if(flags & MAP_STACK) {
			preload = __PAGESIZE;
		} else if(flags & MAP_LAZY) {
			// leave preload == 0
		} else {
			switch(obp->hdr.type) {
			case OBJECT_MEM_ANON:	
				// Anonymous memory up to 8 pages
				preload = 8*__PAGESIZE;
				break;
			case OBJECT_MEM_FD:	
				// ELF data segments up to 4 pages
				if(((flags & (PROT_WRITE|MAP_ELF|MAP_TYPE))==(PROT_WRITE|MAP_ELF|MAP_PRIVATE))) {
					preload = 4*__PAGESIZE;
				}
				break;
			case OBJECT_MEM_SHARED:	
				if(obp->mem.mm.flags & SHMCTL_HIGHUSAGE) {
					/* wired objects should be setup right away */
					preload = size;
				} else if(obp->mem.mm.flags & SHMCTL_LAZY) {
					// leave preload as zero
				} else if((flags & PROT_WRITE|MAP_TYPE) == (PROT_WRITE|MAP_SHARED)) {
					// Writable shared memory up to 8 pages
					preload = 8*__PAGESIZE;
				}
				break;
			default: 
				break;
			}
			if((preload > size) || (size <= __PAGESIZE)) {
				preload = size;
			}
		}
	}
	if(adp->flags & MM_ASFLAG_LOCKALL) {
		r = ms_lock(adp, &ms);
		if(r == EOK) {
			if((flags & MAP_LAZY) && (preload != 0)) {
				r = setup_mappings(prp, &ms, flags, preload);
			}
#if defined(CPU_GBL_VADDR_START)
			else if(extra_flags & EXTRA_FLAG_GBL_VADDR) {
				// Make sure we tell CPU specific code about the mapping
				// if we didn't do so via the setup_mappings() above
				switch(obp->hdr.type) {
				case OBJECT_MEM_SHARED:
				case OBJECT_MEM_TYPED:
					(void)cpu_gbl_mmap(adp, ms.first->start, ms.last->end, obp->mem.mm.flags);
					break;
				default:
					break;
				}
			}
#endif
		}
	} else if(!(flags & MAP_LAZY) || (preload != 0)) {
		r = setup_mappings(prp, &ms, flags, preload);
	}

	//RUSH2: What if obj == anmem_obp on entry (think "/proc/self/as")
	if((obp == anmem_obp) && ANMEM_MULTI_REFS(obp)) {
		memobj_unlock(obp);
	}

	if(r != EOK) goto fail7;
	
	size += guardsize;
	if (extra_flags & EXTRA_FLAG_RLIMIT_DATA) {
		adp->rlimit.data += size;
	}
	adp->rlimit.vmem += size;

	// Make sure we tell the memmgr we're done with the mm. This will
	// also coalesce this mapping with any adjacent ones if possible.
	map_coalese(&ms);

	*vaddrp = (void *)(vaddr + pg_offset);
#if defined(LOADER_BSS_KLUDGE)
	size += kludge_size;
#endif	
	*sizep = size - pg_offset;

	return EOK;

fail7:
	(void) ms_unmap(adp, &ms, UNMAP_NORLIMIT);
	return r;

fail6:	
	{
		struct mm_map	*del;

		for(del = ms.first; del != mm; del = del->next) {
			if(del->obj_ref != NULL) {
				memref_del(del);
			}
		}
	}
	map_remove(&ms);

fail5:
	if(anmem_off != ~(off64_t)0) {
		memobj_lock(anmem_obp);
		memobj_pmem_del_len(anmem_obp, anmem_off, size);
		memobj_unlock(anmem_obp);
	}

fail4:
	if(obp->hdr.type == OBJECT_MEM_TYPED) {
		// Free the memory allocated in tymem_pmem_alloc()
		for(mm = ms.first; mm != ms.last->next; mm = mm->next) {
			memobj_pmem_del_len(obp, mm->offset, ( mm->end - mm->start ) + 1);
		}
	} else if(flags & MAP_ANON) {
		memobj_pmem_del_len(obp, boff, size);
	}
	if((pq != NULL) && (pq->blk == PAQ_BLK_FAKE)) {
		// This is actually a struct pa_quantum_fake
		pa_free_fake(pq);
	}

fail3:
	if(flags & (MAP_ANON|MAP_PHYS)) {
		anmem_unoffset(obp, boff, size);
	}

fail2:	
	map_destroy(&ms);
	if(ms_repl.first != NULL) map_coalese(&ms_repl);

	if((obp == anmem_obp) && ANMEM_MULTI_REFS(obp)) {
		memobj_unlock(obp);
	}

fail1:	
#if defined(CPU_GBL_VADDR_START)
	//RUSH3: CPUism. This code is only good for ARM. See previous 
	//RUSH3: CPU_GBL_VADDR_START conditional section for details.
	if((global_vaddr != VA_INVALID) && !(obp->mem.mm.flags & SHMCTL_GLOBAL)) {
		(void) gbl_vaddr_unmap(global_vaddr, ROUNDUP(size_requested, __PAGESIZE));
	}
#endif	
fail0:	
#ifdef LOADER_BSS_KLUDGE	
	if(kludge_vaddr != NULL) {
		(void) vmm_munmap(prp, (uintptr_t)kludge_vaddr, kludge_size, 0, kludge_mpart_id);
	}
#endif	
	return r;
}

__SRCVERSION("vmm_mmap.c $Rev: 211761 $");
