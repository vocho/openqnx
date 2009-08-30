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
#include "procmgr_internal.h"

static unsigned thread_stack_prot = PROT_READ|PROT_WRITE|PROT_EXEC;

int 
procmgr_stack(message_context_t *ctp, int code, unsigned flags, void *handle) {
	union sigval					value = ctp->msg->pulse.value;
	PROCESS							*prp;
	pid_t							pid;
	int								tid;
	struct _thread_attr				attr;
	int								status;


	if(StackWaitInfo(value, &pid, &tid, &attr) == -1) {
		return 0;
	}
	status = EL2HLT;
	if((prp = proc_lookup_pid(pid))) {
		part_id_t mpid = mempart_getid(prp, sys_memclass_id);

		if(proc_wlock_adp(prp) != 0) {
			crash();
		}
		ProcessBind(prp->pid);

		if(attr.__guardsize == ~0U) {
			(void)memmgr.munmap(prp, (uintptr_t)attr.__stackaddr, attr.__stacksize, 0, mpid);
			status = EOK;
		} else {
			unsigned		mmap_flags;

			mmap_flags = MAP_PRIVATE | MAP_ANON | MAP_STACK | MAP_NOINIT;
			if(!(attr.__flags & PTHREAD_STACK_NOTLAZY)) {
				mmap_flags |= MAP_LAZY;
			}
			status = memmgr.mmap(prp, 0, attr.__stacksize + attr.__guardsize, 
					thread_stack_prot, mmap_flags, 0, 
					attr.__guardsize, __PAGESIZE, attr.__prealloc, NOFD,
					&attr.__stackaddr, &attr.__stacksize, mpid);
			if(status == ENOMEM) {
				status = EAGAIN;
			}
		}

		ProcessBind(0);
		proc_unlock_adp(prp);
	}

	(void)StackCont(pid, tid, status, &attr);
	return 0;
}


void
procmgr_stack_executable(int enable) {
	if(enable) {
		thread_stack_prot |= PROT_EXEC;
	} else {
		thread_stack_prot &= ~PROT_EXEC;
	}
}

__SRCVERSION("procmgr_stack.c $Rev: 169350 $");
