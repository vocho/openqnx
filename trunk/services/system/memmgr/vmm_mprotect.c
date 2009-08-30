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


int 
vmm_mprotect(PROCESS *prp, uintptr_t vaddr, size_t len, int prot) {
	ADDRESS					*as;
	int						r;
	struct map_set			ms;
	struct mm_map			*mm;
	unsigned				old;
	unsigned				cache_flags;
	struct mm_object_ref	*or;
	OBJECT					*obp;
	uintptr_t				end_vaddr;

	as = prp->memory;
	r = map_isolate(&ms, &as->map, vaddr, len, MI_SPLIT);
	if(r != EOK) goto fail1;
//START KLUDGE
	// We need to check the vaddr for being page aligned, but only
	// if MAP_ELF isn't on. See memmgr_ctrl.c where it calls 
	// memmgr.mprotect for details.
	if(!(ms.first->mmap_flags & MAP_ELF) && (mm_flags & MM_FLAG_ENFORCE_ALIGNMENT)) {
		if(ADDR_OFFSET(vaddr) != 0) {
			r = EINVAL;
			goto fail2;
		}
	}
//END KLUDGE	

	// CacheControl() might cause page faults, so let fault_pulse() 
	// know that it doesn't have to grab the lock for this reference
	proc_lock_owner_mark(prp);

	end_vaddr = ms.last->end;
	mm = ms.first;
	for( ;; ) {
		if((mm->extra_flags & EXTRA_FLAG_RDONLY) && (prot & PROT_WRITE)) {
			r = EACCES;
			goto fail2;
		}
		or = mm->obj_ref;
		if(or != NULL) {
			obp = or->obp;
			old = mm->mmap_flags & PROT_MASK;
			if(old != prot) {
				mm->mmap_flags = (mm->mmap_flags & ~PROT_MASK) | prot;
				memobj_lock(obp);
				r = memory_reference(&mm, mm->start, mm->end, MR_NOINIT, &ms);
				memobj_unlock(obp);
				if(r != EOK) goto fail2;
				//RUSH3: Can these be moved into memory_reference()?
				cache_flags = 0;
				if(!(old & PROT_NOCACHE) && (prot & PROT_NOCACHE)) {
					cache_flags |= MS_SYNC|MS_INVALIDATE;
					if(old & PROT_EXEC) {
						cache_flags |= MS_INVALIDATE_ICACHE;
					}
				}
				if((old & PROT_WRITE) && (prot & PROT_EXEC)) {
					cache_flags |= MS_INVALIDATE_ICACHE;
				}
				if(cache_flags != 0) {
					CPU_CACHE_CONTROL(as, (void *)mm->start, (mm->end-mm->start)+1, cache_flags);
				}
			}
		}
		if(mm->end >= end_vaddr) break;
		mm = mm->next;
	}
fail2:
	map_coalese(&ms);
fail1:	
	return r;
}

__SRCVERSION("vmm_mprotect.c $Rev: 161772 $");
