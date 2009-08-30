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

static unsigned pulse_code_lock;

static int
do_lock(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	do {
		pq->flags |= PAQ_FLAG_LOCKED;
		++pq;
	} while(--num > 0);
	return EOK;
}

int
ms_lock(ADDRESS *adp, struct map_set *ms) {
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	OBJECT					*obp;
	int						haslock;
	int						r;
	PROCESS					*prp;
	size_t					size;
	uintptr_t				end_vaddr;

	CRASHCHECK(adp == NULL);
	prp = object_from_data(adp, address_cookie);
	end_vaddr = ms->last->end;
	mm = ms->first;
	for( ;; ) {
		or = mm->obj_ref;
		if(or != NULL) {
			size = (mm->end - mm->start) + 1;
			if(rlimit_blown(prp, RLIMIT_MEMLOCK, adp->rlimit.memlock + size)) {
				return EAGAIN;
			}
			mm->extra_flags |= EXTRA_FLAG_LOCK;
			obp = or->obp;
			haslock = memobj_cond_lock(obp);
			r = memobj_pmem_walk_mm(MPW_SYSRAM, mm, do_lock, NULL);
			//If it's a MAP_LAZY reference, don't force the memory
			//in - user has to explicitly touch it.
			if((r == EOK) && !(mm->mmap_flags & MAP_LAZY) && !SHM_LAZY(obp)) {
				r = memory_reference(&mm, mm->start, mm->end, MR_TRUNC, ms);
			}
			if(!haslock) memobj_unlock(obp);
			if(r != EOK) return r;
			adp->rlimit.memlock += size;
		}
		if(mm->end >= end_vaddr) break;
		mm = mm->next;
	}
	return EOK;
}

int
vmm_mlock(PROCESS *prp, uintptr_t vaddr, size_t len, int flags) {
	ADDRESS					*adp;
	int						r;
	struct map_set			ms;

	adp = prp->memory;

	if(flags == -1) {
		//Somebody's attaching an ISR. We have to make sure that the
		//aspace is fully locked and loaded.
		if((adp->flags & MM_ASFLAG_ISR_LOCK) && !(adp->flags & MM_ASFLAG_ISR_INPROGRESS)) {
			return EOK;
		}
		(void)PageWait(0, 0, prp->pid, pulse_code_lock);
		return -1;
	}

	if(flags & MCL_FUTURE) adp->flags |= MM_ASFLAG_LOCKALL;

	if(!(flags & MCL_CURRENT)) return EOK;

	r = map_isolate(&ms, &adp->map, vaddr, len, MI_SPLIT);
	if(r == EOK) {
		r = ms_lock(adp, &ms);
		map_coalese(&ms);
	}
	return r;
}


static int 
lock_pulse(message_context_t *ctp, int code, unsigned _flags, void *handle) {
	pid_t				pid;
	pid_t				aspace_pid;
	int					tid;
	uintptr_t			vaddr;
	PROCESS				*prp;
	ADDRESS				*adp;
	union sigval		value;
	unsigned			sigcode;
	int					r;

	r = EINVAL;
	value = ctp->msg->pulse.value;
	vaddr = PageWaitInfo(value, &pid, &tid, &aspace_pid, &sigcode);
	if(vaddr == (uintptr_t)-1) goto fail1;

	prp = proc_lookup_pid(aspace_pid);
	if(prp == NULL) goto fail1;

	adp = prp->memory;
	if(proc_wlock_adp(prp)) goto fail1;
	if(!(adp->flags & MM_ASFLAG_ISR_LOCK)) {
		adp->flags |= MM_ASFLAG_ISR_LOCK | MM_ASFLAG_ISR_INPROGRESS;

		ProcessBind(aspace_pid);

		r = vmm_mlock(prp, 0, ~0, MCL_CURRENT|MCL_FUTURE);

		if(r != EOK) {
			adp->flags &= ~(MM_ASFLAG_ISR_LOCK | MM_ASFLAG_ISR_INPROGRESS);
			goto fail2;
		}

		adp->flags &= ~MM_ASFLAG_ISR_INPROGRESS;
	}
	r = 0;

fail2:
	ProcessBind(0);
	proc_unlock_adp(prp);

fail1:
	(void)PageContErrno(pid, tid, r);
	return 0;
}


void
lock_init(void) {
	pulse_code_lock = pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, lock_pulse, NULL);
}

__SRCVERSION("vmm_mlock.c $Rev: 172504 $");
