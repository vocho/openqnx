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


static int
clear_bad_pages(struct mm_object_ref *or, struct mm_map *mm, void *d) {
	size_t		oldsize = *(uint64_t *)d;
	size_t		end;
	size_t		grow;
	size_t		map_size;

	map_size = (mm->end - mm->start) + 1;
	end = mm->offset + map_size;
	if(end > oldsize) {
		grow = end - oldsize;
		if(grow > map_size) {
			grow = map_size;
		}
		// Turn off any 'badness' indicator in the PTE that might have
		// been set by pte_bad().
		pte_unmap(or->adp, (mm->end - grow) + 1, mm->end, or->obp);
	}
	return EOK;
}


static int
shrink_ref(struct mm_object_ref *or, struct mm_map *mm, void *d) {
	size_t		newsize = *(uint64_t *)d;
	size_t		end;
	size_t		shrink;
	size_t		map_size;

	map_size = (mm->end - mm->start) + 1;
	end = mm->offset + map_size;
	if(end > newsize) {
		shrink = end - newsize;
		if(shrink > map_size) {
			shrink = map_size;
		}
		pte_unmap(or->adp, (mm->end - shrink) + 1, mm->end, or->obp);
	}
	return EOK;
}

static int
do_zero(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	size_t		pg_off = *(size_t *)d & (QUANTUM_SIZE-1);
	void		*vaddr;
	size_t		size;
	int			r;

	// If the quantum's been modified, we have to zero the 
	// portion beyond end of the object so that accesses don't pick up 
	// whatever content was there before.
	if(pq->flags & PAQ_FLAG_MODIFIED) {
		r = vmm_mmap(sysmgr_prp, 0, QUANTUM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, obp,
				off, 0, QUANTUM_SIZE, NOFD, &vaddr, &size, obp->hdr.mpid);
		if(r != EOK) return r;
		memset((uint8_t *)vaddr + pg_off, 0, QUANTUM_SIZE - pg_off);
		//Flush the cache in case somebody's got it mmap'd PROT_NOCACHE
		CPU_CACHE_CONTROL(sysmgr_prp->memory, vaddr, size, MS_SYNC);
		(void)vmm_munmap(sysmgr_prp, (uintptr_t)vaddr, size, 0, obp->hdr.mpid);
	}
	return EOK;
}


static int
zero_tail(OBJECT *obp, size_t size) {
	pid_t	old;
	size_t	pg_off;
	int		r;

	old = ProcessBind(-1);
	if(old != 0) ProcessBind(0);
	ProcessBind(SYSMGR_PID);
	proc_wlock_adp(sysmgr_prp);
	memobj_lock(obp);
	pg_off = size & ~(QUANTUM_SIZE-1);
	r = memobj_pmem_walk(MPW_SYSRAM, obp, pg_off, pg_off, do_zero, &size);
	memobj_unlock(obp);
	proc_unlock_adp(sysmgr_prp);
	ProcessBind(old);
	return r;
}


int
vmm_resize(OBJECT *obp, size_t size) {
	uint64_t			newsize;
	uint64_t			oldsize;
	unsigned			flags;
	int					r;
	int					has_lock;
	uintptr_t			vaddr;

	flags = MAP_ANON;
	switch(obp->hdr.type) {
	case OBJECT_MEM_SHARED:	
		if(obp->mem.mm.flags & MM_SHMEM_SPECIAL) {
			switch(obp->mem.mm.flags & (SHMCTL_ANON|SHMCTL_PHYS)) {
			case 0:
			case SHMCTL_ANON:
				break;
			case SHMCTL_PHYS:
				flags = 0;
				break;
			case SHMCTL_ANON|SHMCTL_PHYS:
				flags |= MAP_PHYS;
				break;
			default:
				break;
			}
			if(obp->mem.mm.flags & SHMCTL_ISADMA) {
				//RUSH3: CPUism
#ifdef __X86__			
				flags |= MAP_ANON|MAP_BELOW16M|MAP_NOX64K|MAP_PHYS;
#else				
				flags |= MAP_ANON|MAP_PHYS;
#endif
			}
			if(obp->mem.mm.flags & SHMCTL_NOX64K) {
				flags |= MAP_NOX64K;
			}
		}
		vaddr = obp->shmem.vaddr;
		break;
	case OBJECT_MEM_FD:
		vaddr = VA_INVALID;
		break;
	//RUSH2: Explicitly listed here so we can find it if we decide to
	//RUSH2: make OBJECT_MEM_TYPED an OBJECT_MEM_SHARED with a MM_MEM_TYPED
	//RUSH2: flag bit. In that case we'll have to add gear to the OBJECT_MEM_SHARED
	//RUSH2: case above
	case OBJECT_MEM_TYPED: 
		// fall through
	default:
		return EINVAL;
	}

	has_lock = proc_mux_haslock(&obp->mem.mm.mux, 0);
	if(!has_lock) {
		if(((size & (QUANTUM_SIZE-1)) != 0) && (size < obp->mem.mm.size)) {
			// non quantum aligned new size and we're shrinking. We
			// need to zero the piece of the last page (if it's already
			// been initialized).
			r = zero_tail(obp, size);
			if(r != EOK) goto fail1;
		}
		memobj_lock(obp);
	}

	oldsize = ROUNDUP(obp->mem.mm.size, __PAGESIZE);
	newsize = ROUNDUP(size, __PAGESIZE);

	if(newsize > oldsize) {
		//growing...
		if(vaddr != VA_INVALID) {
			// Can't grow a global object after it's been assigned an address
			r = ENOMEM;
			goto fail2;
		}
		r = memobj_offset_check(obp, 0, newsize);
		if((r == EOK) && !SHM_LAZY(obp)) {
			r = memobj_pmem_add(obp, oldsize, newsize - oldsize, flags);
		}
		if(obp->mem.mm.flags & MM_MEM_HAS_BAD_PAGE) {
			memref_walk(obp, clear_bad_pages, &oldsize);
			obp->mem.mm.flags &= ~MM_MEM_HAS_BAD_PAGE;
		}
	} else if(newsize < oldsize) {
		//shrinking...
		memref_walk(obp, shrink_ref, &newsize);
		r = memobj_pmem_del_end(obp, newsize, ~(off64_t)0);
		if(vaddr != VA_INVALID) {
			if(GBL_VADDR_UNMAP(vaddr + (uintptr_t)newsize, oldsize - newsize) != EOK) {
				crash();
			}
#if defined(CPU_GBL_VADDR_START)
			//RUSH3: CPUism, ARM specific
			if(obp->mem.mm.flags & SHMCTL_GLOBAL) {
				if(pte_unmap(NULL, vaddr + (uintptr_t)newsize, vaddr + (uintptr_t)(oldsize - newsize) - 1, obp) != EOK) {
					crash();
				}
			}
#endif			
		}
	} else {
		r = EOK;
	}
	if((newsize == 0) && (obp->mem.mm.pmem != NULL)) crash();
	if(r != EOK) goto fail2;

	obp->mem.mm.size = size;
	if(!has_lock) memobj_unlock(obp);
	return EOK;

fail2:	
	if(!has_lock) memobj_unlock(obp);

fail1:	
	return r;
}

__SRCVERSION("vmm_resize.c $Rev: 211761 $");
