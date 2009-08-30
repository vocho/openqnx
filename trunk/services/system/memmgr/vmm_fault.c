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
vmm_fault(struct fault_info *info) {
	ADDRESS			*adp;
	struct map_set	ms;
	struct mm_map	*mm;
	int				r;

	r = cpu_vmm_fault(info);

	if(r == 0) {
		adp = info->prp->memory;
		if(adp == NULL) crash();

		if(map_fault_lock(&adp->map)) {
			if(!(adp->flags & MM_ASFLAG_PRIVATIZING)) {
				map_isolate(&ms, &adp->map, info->vaddr, 0, MI_NONE);
				mm = ms.first;
				if(mm == NULL) {
					r = -1;
				} else if(info->sigcode & SIGCODE_STORE) {
					if(!(mm->mmap_flags & PROT_WRITE)) r = -1;
				} else if((info->sigcode & 0xffff) == MAKE_SIGCODE(SIGSEGV, SEGV_ACCERR, 0)) {
					//If it's a read/instr fetch and we got an SEGV_ACCERR,
					//we know the reference is no good.
					r = -1;
				} else {
					//Got a SEGV_MAPERR, it might just be that we haven't
					//bothered setting up the pte's yet.
					if(!(mm->mmap_flags & (PROT_READ|PROT_EXEC))) r = -1;
				}
				map_coalese(&ms);
			}
			map_fault_unlock(&adp->map);
		}
	} else if(r == -2) {
		// Forced bad page
		info->sigcode = MAKE_SIGCODE(SIGBUS, BUS_OBJERR, FLTPAGE) | 
							(info->sigcode & SIGCODE_FLAGS_MASK);
		r = -1;
	}

	return r;
}


static int 
fault_pulse(message_context_t *ctp, int code, unsigned _flags, void *handle) {
	pid_t				pid;
	pid_t				aspace_pid;
	int					tid;
	uintptr_t			vaddr, vend;
	PROCESS				*prp;
	ADDRESS				*adp;
	union sigval		value;
	unsigned			sigcode;
	struct map_set		ms;
	struct mm_map		*mm;
	OBJECT				*obp;
	int					r;
	off64_t				off_page_ref;
	int					have_adp_lock;
	int					have_obj_lock;
	int					access_error;
	unsigned			sigcode_flags;
	struct loader_context	*lcp;

	value = ctp->msg->pulse.value;
	vaddr = PageWaitInfo(value, &pid, &tid, &aspace_pid, &sigcode);
	if(vaddr == (uintptr_t)-1) goto fail1;

	prp = proc_lookup_pid(aspace_pid);
	if(prp == NULL) goto fail2;
	adp = prp->memory;
	if(adp == NULL) goto fail2;

	sigcode_flags = sigcode & SIGCODE_FLAGS_MASK;
	have_adp_lock = proc_lock_owner_check(prp, pid, tid);
	if(!have_adp_lock) {
		r = proc_wlock_adp(prp);
		if(r != 0) {
			sigcode = MAKE_SIGCODE(SIGBUS, BUS_ADRERR, FLTSTACK) | sigcode_flags;
			goto fail2;
		}
	}

	ProcessBind(aspace_pid);

	access_error = ((sigcode & 0xffff) == MAKE_SIGCODE(SIGSEGV, SEGV_ACCERR, 0));

	map_isolate(&ms, &adp->map, vaddr, 0, MI_NONE);
	mm = ms.first;
	if(mm == NULL) {
		goto fail4;
	} else if(sigcode & SIGCODE_STORE) {
		if(!(mm->mmap_flags & PROT_WRITE)) goto fail4;
	} else if(access_error) { 
		goto fail4;
	} else {
		if(!(mm->mmap_flags & (PROT_READ|PROT_EXEC))) goto fail4;
	}
	obp = mm->obj_ref->obp;
	if(have_adp_lock) {
		have_obj_lock = proc_mux_haslock(&obp->mem.mm.mux, KerextSyncOwner(pid, tid));
	} else {
		have_obj_lock = 0;
	}
	if(have_obj_lock) {
		// If the faulting thread already had the object locked, we have
		// to disable the VERIFY_OBJ_LOCK() checks since this thread
		// won't own the mutex
#ifndef NDEBUG		
		obp->mem.mm.flags |= MM_MEM_SKIPLOCKCHECK;
#endif		
	} else {
		memobj_lock(obp);
	}

	off_page_ref = mm->offset + (ROUNDDOWN(vaddr, __PAGESIZE) - mm->start);
	if(off_page_ref >= obp->mem.mm.size) {
		// Referencing beyond the end of the object
		sigcode = MAKE_SIGCODE(SIGBUS, BUS_OBJERR, FLTPAGE) | sigcode_flags;
		goto fail5;
	}
	if(!access_error && (mm->extra_flags & EXTRA_FLAG_PRIMARY_STK)) {
		//RUSH3: How much pmem are we going to bring in?
		if(rlimit_blown(prp, RLIMIT_STACK, adp->rlimit.stack + QUANTUM_SIZE)) {
			sigcode = MAKE_SIGCODE(SIGSEGV, SEGV_STKERR, FLTSTACK) | sigcode_flags;
			goto fail5;
		}
	}
	//FUTURE: Take advantage of "mm->extra_flags & EXTRA_FLAG_MADV_MASK" 
	//FUTURE: (madvise) information to gang page things in. Should I have 
	//FUTURE: logic here, or in memory_reference()?

	//RUSH3: If SIGCODE_INXFER is on, we're (probably) in a message
	//RUSH3: pass and should figure out how long the message is and
	//RUSH3: adjust vend for that length
	vend = vaddr;
	if(!(mm->mmap_flags & MAP_LAZY)) {
		switch(obp->hdr.type) {
		case OBJECT_MEM_SHARED:	
			// Only bring in one page for lazily allocated shared objects
			if(obp->mem.mm.flags & SHMCTL_LAZY) break;
			// fall through
		case OBJECT_MEM_ANON:	
			// Fault for an anon/shared page. Let's see if we should bring in
			// more than one page at a time.
			vend = mm->end;
			#define MAX_FAULT_IN 	((16*__PAGESIZE)-1)
			if(vend - vaddr > MAX_FAULT_IN) {
				vend = vaddr + MAX_FAULT_IN;
			}
			break;
		default:
			break;
		}
	}

	r = memory_reference(&mm, vaddr, vend, 
			(sigcode & SIGCODE_STORE) ? (MR_WRITE|MR_TRUNC) : MR_TRUNC, &ms);
	lcp = prp->lcp;
	if((lcp != NULL) && (r != EOK)) {
		lcp->fault_errno = r;
	}
	//RUSH3: Should we create new BUS_? value(s) so that users can
	//RUSH3: tell what failure the memory_reference() got? E.g. no memory?
	//RUSH3: There's a 'si_errno' field in the siginfo_t that would be
	//RUSH3: appropriate, but we have to figure out how to get the information
	//RUSH3: from here into it. If we do that, we can get rid of the
	//RUSH3: lcp->fault_errno stuff just above.
	if(r != EOK) {
		sigcode = MAKE_SIGCODE(SIGBUS, BUS_OBJERR, FLTPAGE) | sigcode_flags;
		goto fail5;
	}

	if(!access_error && (mm->extra_flags & EXTRA_FLAG_PRIMARY_STK)) {
		//RUSH3: How much pmem did we bring in?
		adp->rlimit.stack += QUANTUM_SIZE;
	}
	sigcode = 0;
	
fail5:
	if(sigcode & SIGCODE_INXFER) {
		// We were in a msg pass when the fault occured. Mark the
		// page as bad to avoid an infinite loop
		(void)pte_bad(adp, vaddr);
		obp->mem.mm.flags |= MM_MEM_HAS_BAD_PAGE;
	}
	if(have_obj_lock) {
		// Re-enable VERIFY_OBJ_LOCK() checks
#ifndef NDEBUG		
		obp->mem.mm.flags &= ~MM_MEM_SKIPLOCKCHECK;
#endif		
	} else {
		memobj_unlock(obp);
	}

fail4:
	map_coalese(&ms);

//fail3:
	ProcessBind(0);
	if(!have_adp_lock) proc_unlock_adp(prp);

fail2:
	(void)PageCont(pid, tid, sigcode);

fail1:
	return 0;
}


void
fault_init(void) {
	memmgr.fault_pulse_code = pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, fault_pulse, NULL);
}

__SRCVERSION("vmm_fault.c $Rev: 172504 $");
