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
#include <sys/memmsg.h>

struct kerargs_page_cont {
	pid_t		pid;
	int			tid;
	int			code_signo;
	int			err;
};

static void
kerext_page_cont(void *data) {
	struct kerargs_page_cont *kap = data;
	PROCESS	*prp;
	THREAD	*act = actives[KERNCPU], *thp;

	// Verify the target process and thread exists.
	if((prp = lookup_pid(kap->pid)) == NULL  ||
	   (thp = vector_lookup(&prp->threads, kap->tid-1)) == NULL  ||
	   (thp->state != STATE_WAITPAGE)) {
		kererr(act, ESRCH);
		return;
	}

	lock_kernel();
	if(kap->err != EOK) {
		kererr(thp, kap->err);
	} else if(kap->code_signo) {
		if(kap->code_signo & SIGCODE_KERNEL) {
			if((kap->code_signo & SIGCODE_INXFER) == 0) {
				// Fault occured while we were in a kernel call. Unwind properly
				if(!(kap->code_signo & SIGCODE_KEREXIT)) {
					// Fixup ip
					SETKIP(thp,KIP(thp) + KER_ENTRY_SIZE);
				}
				if(thp->flags & _NTO_TF_WAAA) {
					thp->status = (void *)EAGAIN;
					thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
				} else {
					kererr(thp, EFAULT);
				}
			} else {
				// Else we just restart kernel call, this time it will unwind properly
			}
		} else {
			// Make sure that the signal will be delivered by
			// knocking down all higher priority specret flags,
			// except for KILLSELF and TO_BE_STOPPED
			thp->flags &= ~(_NTO_TF_SIG_ACTIVE - 1) | (_NTO_TF_KILLSELF|_NTO_TF_TO_BE_STOPPED);

			// Deliver the signal
			// PageWait() stored the faulting address in "thp->next"
			usr_fault(kap->code_signo, thp, (uintptr_t)thp->next.thread);
		}
	}

	ready(thp);
	SETKSTATUS(act, EOK);
}


int
PageCont(pid_t pid, int tid, int code_signo) {
	struct kerargs_page_cont	data;

	data.pid = pid;
	data.tid = tid;
	data.code_signo = code_signo;
	data.err = EOK;
	return __Ring0(kerext_page_cont, &data);
}


int
PageContErrno(pid_t pid, int tid, int err) {
	struct kerargs_page_cont	data;

	data.pid = pid;
	data.tid = tid;
	data.err = err;
	data.code_signo = 0;
	return __Ring0(kerext_page_cont, &data);
}


int
PageFaultWait(struct fault_info *info) {
	int		r;

	r = PageWait(info->vaddr, info->sigcode, info->prp->pid,
					memmgr.fault_pulse_code);
	if(r != EOK) {
		/*
		 * We're in serious trouble here. We either got called from an
		 * interrupt handler or we've run out of memory. Return a
		 * somewhat strange code so that people know what's going on.
		 */
		info->sigcode = MAKE_SIGCODE(SIGILL, ILL_BADSTK, FLTSTACK);
	}
	return r;
}


int
PageWait(uintptr_t vaddr, unsigned flags, pid_t pid, int code) {
	THREAD				*thp = actives[KERNCPU];
	struct sigevent		event;

#ifndef VARIANT_smp
	if(get_inkernel() & INKERNEL_INTRMASK) {
		return EINVAL;
	}
#endif

//RUSH3: See about turning this back on after we've fiddled the kernel.s
//RUSH3: code for the new vmm_fault() interface.
#if !defined(VARIANT_smp) || !defined(__PPC__)
CRASHCHECK(!(get_inkernel() & INKERNEL_LOCK));
#endif
	CRASHCHECK(thp->state != STATE_RUNNING);

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = MEMMGR_COID;
	event.sigev_value.sival_int = SYNC_OWNER(thp);
	event.sigev_priority = thp->priority;
	event.sigev_code = code;

	if(thp->sched_flags & AP_SCHED_RUNNING_CRIT) {
		SIGEV_MAKE_CRITICAL(&event);
	}

	if(sigevent_proc(&event)) {
		// Pulse failed, leave the thread ready so the signal is delivered
		return -1;
	}

	// we may not be running after calling sigevent_proc()
	unready(thp, STATE_WAITPAGE);

	// OVERLOADING next, prev, status
	thp->next.thread = (void *)vaddr;
	thp->prev.thread = (void *)pid;
	thp->status      = (void *)flags;

	return 0;
}

struct kerargs_page_wait_info {
	unsigned 		value;
	pid_t			*ppid;
	int				*ptid;
	pid_t			*paspace_pid;
	unsigned		*pflags;
	uintptr_t		vaddr;
};

static void
kerext_page_wait_info(void *data) {
	struct kerargs_page_wait_info	*kap = data;
	int								id;
	unsigned						index;
	pid_t							pid;
	PROCESS							*prp;
	THREAD							*thp;
	unsigned						pt_info;

	pt_info = kap->value;
	id = SYNC_PINDEX(pt_info);
	if(id < process_vector.nentries  &&
			(prp = VECP(prp, &process_vector, id))  &&
			id == PINDEX(prp->pid) &&
			(thp = vector_lookup(&prp->threads, SYNC_TID(pt_info))) &&
			(thp->state == STATE_WAITPAGE)) {
		kap->vaddr = (uintptr_t)thp->next.thread;
		*kap->ppid = prp->pid;
		*kap->ptid = thp->tid + 1;
		*kap->pflags = (unsigned)thp->status;

		pid = (unsigned)thp->prev.thread;
		index = PINDEX(pid);
		if((index >= process_vector.nentries)
			|| ((prp = VECP(prp, &process_vector, index)) == NULL)
			|| (prp->pid != pid)) {
			pid = 0;
		}
		*kap->paspace_pid = pid;
	} else {
		kap->vaddr = ~0;
	}
}

void
waitpage_status_get(THREAD *thp, debug_thread_t *dtp) {
    struct kerargs_page_wait_info  ka;
	pid_t                          aspid;  /* place holder */
	int                            tid;    /* place holder */

	ka.value       = SYNC_OWNER_BITS((unsigned) thp->prev.thread, thp->tid);
	ka.ppid        = &dtp->blocked.waitpage.pid;
	ka.ptid        = &tid;
	ka.paspace_pid = &aspid;
	ka.pflags      = &dtp->blocked.waitpage.flags;

	dtp->blocked.waitpage.pid = 0;   /* Initialize */
	kerext_page_wait_info(&ka);

	dtp->blocked.waitpage.vaddr = ka.vaddr;

	return;
}

uintptr_t
PageWaitInfo(union sigval value, pid_t *ppid, int *ptid, pid_t *paspace_pid, unsigned *pflags) {
	struct kerargs_page_wait_info		data;

	data.value = value.sival_int;
	data.ppid = ppid;
	data.ptid = ptid;
	data.paspace_pid = paspace_pid;
	data.pflags = pflags;
	__Ring0(kerext_page_wait_info, &data);
	return data.vaddr;
}

__SRCVERSION("kerext_page.c $Rev: 169426 $");
