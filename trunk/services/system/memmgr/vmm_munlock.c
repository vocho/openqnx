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

struct lock_data {
	off64_t		start;
	off64_t		end;
	unsigned	flag;
};

static int
fiddle_lock(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct lock_data	*data = d;

	do {
		pq->flags = (pq->flags & ~PAQ_FLAG_LOCKED) | data->flag;
		++pq;
	} while(--num > 0);
	return EOK;
}


static int
relock(struct mm_object_ref *or, struct mm_map *mm, void *d) {
	struct lock_data	*data = d;
	off64_t				start;
	off64_t				end;
	int					r;

	if(mm->extra_flags & EXTRA_FLAG_LOCK) {
		start = mm->offset;
		end = start + (mm->end - mm->start);
		if(start < data->start) start = data->start;
		if(end > data->end) end = data->end;
		if(start < end) {
			r = memobj_pmem_walk(MPW_SYSRAM, mm->obj_ref->obp, start, end, 
									fiddle_lock, data);
			if(r != EOK) return r;
		}
	}
	return EOK;
}

int
vmm_munlock(PROCESS *prp, uintptr_t vaddr, size_t len, int flags) {
	ADDRESS					*adp;
	int						r;
	struct map_set			ms;
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	OBJECT					*obp;
	struct lock_data		data;

	adp = prp->memory;

	if(flags & MCL_FUTURE) {
		adp->flags &= ~MM_ASFLAG_LOCKALL;
	}

	if(!(flags & MCL_CURRENT)) {
		return EOK;
	}

	r = map_isolate(&ms, &adp->map, vaddr, len, MI_SPLIT);
	if(r != EOK) goto fail1;

	for(mm = ms.first; mm != ms.last->next; mm = mm->next) {
		or = mm->obj_ref;
		if(or != NULL) {
			mm->extra_flags &= ~EXTRA_FLAG_LOCK;
			obp = or->obp;
			memobj_lock(obp);

			// Turn off the PAQ_FLAG_LOCKED bit, then run around all the
			// references and turn it back on if there are any other
			// locked mmap's.

			data.flag = 0;
			r = memobj_pmem_walk_mm(MPW_SYSRAM, mm, fiddle_lock, &data);
			if(r != EOK) goto fail2;
			data.start = mm->offset;
			data.end = mm->offset + (mm->end - mm->start);
			data.flag = PAQ_FLAG_LOCKED;
			r = memref_walk(obp, relock, &data);
			if(r != EOK) {
				// We had a problem relocking, undo & fail the munlock()
				mm->extra_flags |= EXTRA_FLAG_LOCK;
				(void)memobj_pmem_walk_mm(MPW_SYSRAM, mm, fiddle_lock, &data);
				memobj_unlock(obp);
				goto fail2;
			}
			memobj_unlock(obp);
		}
		adp->rlimit.memlock -= (mm->end - mm->start) + 1;
	}

fail2:
	map_coalese(&ms);

fail1:	
	return r;
}

__SRCVERSION("vmm_munlock.c $Rev: 153052 $");
