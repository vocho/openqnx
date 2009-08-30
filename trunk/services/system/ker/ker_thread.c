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

int kdecl
ker_thread_create(THREAD *act,struct kerargs_thread_create *kap) {
	PROCESS				*prp;
	struct sigevent		event;

	// Verify the process to create the thread in exists.
	if(!(prp = (kap->pid ? lookup_pid(kap->pid) : act->process))) {
		return ESRCH;
	}

	if(prp != act->process  &&  act->process->pid != SYSMGR_PID) {
		return EPERM;
	}

	// check that we're not exceed any imposed limits
	if (prp->num_active_threads >= prp->rlimit_vals_soft[RLIMIT_NTHR]) {
		return EINVAL;   // is this the right error code?
	}

	// Make a local copy before locking the kernel so we won't fault latter.
	if(kap->attr) {
		RD_VERIFY_PTR(act, kap->attr, sizeof *kap->attr);
		RD_PROBE_INT(act, kap->attr, sizeof *kap->attr / sizeof(int));
	}
	event.sigev_notify_attributes = kap->attr;
	event.sigev_notify_function = kap->func;
	event.sigev_value.sival_ptr = kap->arg;

	lock_kernel();

	return thread_create(act, prp, &event, THREAD_CREATE_BLOCK_FLAG);
}

static void
create_sigchld(PROCESS *prp, int status) {
	uint64_t	runtime;
	uint64_t	systime;

	if((prp->flags & _NTO_PF_NOZOMBIE) && !(prp->flags & _NTO_PF_LOADING)) {
		return; /* daemon, not change exit status */
	}

	do {
		runtime = prp->running_time;
	} while(runtime != prp->running_time);
	do {
		systime = prp->system_time;
	} while(systime != prp->system_time);
	prp->siginfo.si_utime = (runtime-systime) / (_NTO_BILLION/clk_tck);
	prp->siginfo.si_stime = systime           / (_NTO_BILLION/clk_tck);
	prp->siginfo.si_status = status;
	prp->siginfo.si_signo = SIGCHLD;
	prp->siginfo.si_code = CLD_EXITED;
	prp->siginfo.si_pid = prp->pid;
}

int kdecl
ker_thread_destroy(THREAD *act,struct kerargs_thread_destroy *kap) {
	THREAD *thp;
	PROCESS	*prp = act->process;

	// Code to handle case where Proc is replaced with miniproc and
	// a program in the build file (which is created as a thread of
	// the system process) calls exit. We only let it destroy itself.
	if(kap->tid == -1  &&  prp->pid == SYSMGR_PID) {
		kap->tid = 0;
	}
	if((int)kap->priority > 0 && (int)kap->priority <= NUM_PRI-1) {
		prp->terming_priority = kap->priority;
	} else {
		prp->terming_priority = act->priority;
	}

	// If it is not a multi destroy, verify the target thread exists.
	if(kap->tid != -1) {
		thp = kap->tid ? vector_lookup(&prp->threads, kap->tid-1) : act;
		if(thp == NULL) {
			return ESRCH;
		}
		lock_kernel();

		// A single thread exit may result in the last thread going away in
		// which case the process gets a zero exit status.
		if(prp->num_active_threads > 1 || (prp->threads.nentries-1 == prp->threads.nfree)) {
			create_sigchld(prp, 0);
			thp->status = kap->status;
			thread_destroy(thp);
			return EOK;
		}

		// To get here, this must be the last thread and we have zombies
		// which need to be purged. This is handled by the multi-code below.
	}

	// We are going to take out all threads.
	lock_kernel();
	create_sigchld(prp, (int)kap->status);

	// We force the caller to kill all its sibling threads.
	thread_destroyall(act);
	return ENOERROR;
}


int kdecl
ker_thread_destroyall(THREAD *act,struct kerargs_null *kap) {
	int tid;
	THREAD *thp;
	PROCESS *prp = act->process;

	lock_kernel();
	// If ONLYME is set, thp->status was must be set before setting KILLSELF
	// If there is only one thread left, cleanup any zombies (for race condition in stack de-allocation)
	if((act->flags & _NTO_TF_ONLYME) == 0 || prp->num_active_threads == 1) {
		for(tid = 0 ; tid < prp->threads.nentries ; ++tid) {
			if((thp = VECP2(thp, &prp->threads, tid))  &&  thp != act  &&
					!(prp->pid == SYSMGR_PID  &&  thp->tid < NUM_PROCESSORS)) {
				thp->flags |= (_NTO_TF_DETACHED | _NTO_TF_KILLSELF);
				thp->status = 0;
				thread_destroy(thp);
				act->flags &= ~_NTO_TF_ONLYME;
				KER_PREEMPT(act, ENOERROR);
			}
		}
		act->flags |= _NTO_TF_DETACHED;
		act->status = 0;
	}

	thread_destroy(act);
	return ENOERROR;
}


int kdecl
ker_thread_detach(THREAD *act,struct kerargs_thread_detach *kap) {
	THREAD *thp;

	// Verify the specified thread exists.
	if((thp = (kap->tid ? vector_lookup2(&act->process->threads, kap->tid-1) : act)) == NULL) {
		return ESRCH;
	}

	if(thp->flags & _NTO_TF_DETACHED) {
		return EINVAL;
	}

	lock_kernel();

	// If anyone was waiting to join on me wake them up with an error
	if(thp->join) {
		force_ready(thp->join, EINVAL);
		thp->join = NULL;
	}
	thp->flags |= _NTO_TF_DETACHED;

	if(thp->state == STATE_DEAD) {
		thp->status = 0;
		thread_destroy(thp);
	}

	return EOK;
}


int kdecl
ker_thread_join(THREAD *act,struct kerargs_thread_join *kap) {
	THREAD *thp;
	unsigned	tls_flags;

	// Verify the specified thread exists.
	if((thp = vector_lookup2(&act->process->threads, kap->tid-1)) == NULL) {
		return ESRCH;
	}

	// It is invalid to join a detached thread.
	if(thp->flags & _NTO_TF_DETACHED) {
		return EINVAL;
	}

	// It is invalid to join a thread which is already being joined.
	if(thp->join) {
		return EBUSY;
	}

	// It is invalid to join to yourself.
	if(thp == act) {
		return EDEADLK;
	}

	if(kap->status) {
		WR_VERIFY_PTR(act, kap->status, sizeof(*kap->status));
		WR_PROBE_OPT(act, kap->status, 1);
	}

	// If the thread is not dead and waiting we block on it.
	if(thp->state != STATE_DEAD) {
		tls_flags = act->un.lcl.tls->__flags;

		lock_kernel();
		if(IMTO(act, STATE_JOIN)) {
			return ETIMEDOUT;
		}

		if(PENDCAN(tls_flags)) {
			SETKIP_FUNC(act,act->process->canstub);
			return ENOERROR;
		}

		act->state = STATE_JOIN;
		_TRACE_TH_EMIT_STATE(act, JOIN);
		block();
		act->blocked_on = thp;
		act->args.jo.statusptr = kap->status;
		thp->join = act;
		return ENOERROR;
	}

	// Return thread exit status to caller.
	if(kap->status) {
		*kap->status = thp->status;
	}
	lock_kernel();

	act->timeout_flags = 0;
	thp->flags |= _NTO_TF_DETACHED;
	thp->status = 0;
	thread_destroy(thp);

	return EOK;
}


int kdecl
ker_thread_cancel(THREAD *act,struct kerargs_thread_cancel *kap) {
	THREAD *thp;

	// If tid == 0 then we simply save the cancel stub.
	if(kap->tid == 0) {
		lock_kernel();
		act->process->canstub = kap->canstub;
		return EOK;
	}

	// Verify the specified thread exists.
	if((thp = vector_lookup(&act->process->threads, kap->tid-1)) == NULL) {
		return ESRCH;
	}

	// Check for WAAA as stack (where tls is usually located) may not
	// have a stack yet.
	if(!(thp->flags & _NTO_TF_WAAA)) {
		atomic_set(&thp->un.lcl.tls->__flags, PTHREAD_CANCEL_PENDING);
	}

	lock_kernel();

	thp->process->canstub = kap->canstub;

	// If cancelation is enabled and we are blocked, force thread ready
	// at address of cancelation handler.
	// We also make thread birth a cancellation point.
	if((thp->flags & _NTO_TF_WAAA)  ||
	  ((thp->un.lcl.tls->__flags & PTHREAD_CANCEL_DISABLE) == 0  &&
	   (STATE_CANCELABLE(thp) || (thp->un.lcl.tls->__flags & PTHREAD_CANCEL_ASYNCHRONOUS)))) {
		force_ready(thp, EINTR);
		thread_cancel(thp);
	}

	return EOK;
}


int kdecl
ker_thread_ctl(THREAD *act, struct kerargs_thread_ctl *kap) {
	return kerop_thread_ctl(act, act, kap);
}

#define RMSK_MAX 1
int
kerop_thread_ctl(THREAD *act, THREAD *op, struct kerargs_thread_ctl *kap) {
	int			prev;
	unsigned	new[RMSK_MAX], inherit[RMSK_MAX], *uintp;
	int			r, num, set_inherit, set_rmask;
	struct _thread_name *tn;
	struct _thread_runmask	*tr;
	char				*prev_name;
	int					 prev_name_len;
#if RMSK_MAX > 1
	int			lastmask;
#endif

	lock_kernel();

	/* Give the vendor extension a chance to take first dibs */
	if ( kerop_thread_ctl_hook != NULL ) {
		r = kerop_thread_ctl_hook(act, op, kap);
		if ( r != ENOSYS ) {
			return r;
		}
	}

	switch(kap->cmd) {
	case _NTO_TCTL_IO:
		if(act != op)
			return ENOTSUP;

		if(!kerisroot(act)) {
			return ENOERROR;
		}
		//Check with the memory manager to make sure that
		//it's locked all the memory for the process (so we don't
		//page fault after an InterruptDisable/Lock has been done).
		r = memmgr.mlock(act->process, 0, 0, -1);
		if(r == -1) {
			// The memmgr has blocked this thread and sent
			// a pulse to lock the memory down. Restart the
			// kernel call - when we come in again all should be well.
			KERCALL_RESTART(act);
			return ENOERROR;
		}
		if(r != EOK) return r;
		cpu_thread_priv(act);
		act->flags |= _NTO_TF_IOPRIV;
		break;

	case _NTO_TCTL_ONE_THREAD_HOLD:
		if(!stop_one_thread(op->process, (int)kap->data - 1, _NTO_TF_THREADS_HOLD, 0)) {
			return ESRCH;
		}
		op->flags &= ~(_NTO_TF_TO_BE_STOPPED | _NTO_TF_THREADS_HOLD);
		break;

	case _NTO_TCTL_THREADS_HOLD:
		stop_threads(op->process, _NTO_TF_THREADS_HOLD, 0);
		op->flags &= ~(_NTO_TF_TO_BE_STOPPED | _NTO_TF_THREADS_HOLD);
		break;

	case _NTO_TCTL_ONE_THREAD_CONT:
		if(!cont_one_thread(op->process, (int)kap->data - 1, _NTO_TF_THREADS_HOLD)) {
			return ESRCH;
		}
		break;

	case _NTO_TCTL_THREADS_CONT:
		cont_threads(op->process, _NTO_TF_THREADS_HOLD);
		break;

	case _NTO_TCTL_RUNMASK:
		if(set_runmask(op, (unsigned) kap->data) != 0) {
			return ENOERROR;
		}
		break;
	case _NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT:
		unlock_kernel();

		num = RMSK_SIZE(NUM_PROCESSORS);
#ifndef NDEBUG
		if (num > RMSK_MAX) crash();
#endif

		/*
		 * We read all members of *tr below before
		 * locking so don't need to RD_PROBE_INT().
		 */
		RD_VERIFY_PTR(act, kap->data,
		    sizeof(struct _thread_runmask) + 2 * num * sizeof(unsigned));
		tr = (struct _thread_runmask *)kap->data;

		/*
		 * We force them to initialize tr->num with RMSK_SIZE().
		 * This is so there's no ambiguity as to how many bits
		 * are valid and / or where the second array starts on
		 * return.  This could occur if they were to pass in a
		 * large tr->size.
		 */
		if(tr->size != num) {
			return EINVAL;
		}

		set_rmask = 0;
		set_inherit = 0;
		uintp = (unsigned *)(tr + 1);
#if RMSK_MAX > 1
/*
 * This will have to be propagated to / synced with other areas.
 * - Currently more error checking and validation is done up
 *   front here rather than other areas:
 * - This assumes we take the mask as is rather than an inverted
 *   copy.
 * - set_runmask()
 */
#error NYI
		memcpy(new, uintp, num * sizeof(unsigned));
		memset(new + num, 0x00, (RMSK_MAX - num) * sizeof(unsigned));
		uintp += num;
		memcpy(inherit, uintp, num * sizeof(unsigned));
		memset(inherit + num, 0x00, (RMSK_MAX - num) * sizeof(unsigned));

		for(r = 0; r < num - 1; r++) {
			if(new[r] != 0)
				set_rmask = 1;
			if(inherit[r] != 0)
				set_inherit = 1;
		}

#error This won't work if NUM_PROCESSORS == 32 && r == 0 - most CPU's
#error perform a mask of 0x1f on the shift count, so a shift of 32 becomes
#error a shift of 0.
		lastmask = (1 << (NUM_PROCESSORS - (r * __INT_BITS__))) - 1;

		if (set_rmask == 0 && new[r] != 0) {
			if((new[r] & lastmask) == 0)
				return EINVAL;
			set_rmask = 1;
		}
		if(set_inherit == 0 && inherit[r] != 0) {
			if((inherit[r] & lastmask) == 0)
				return EINVAL;
			set_inherit = 1;
		}
		new[r] &= lastmask;
		inherit[r] &= lastmask;
#else
		new[0] = *uintp;
		uintp++;
		inherit[0] = *uintp;

		if(inherit[0] != 0) {
			if((inherit[0] & LEGAL_CPU_BITMASK) == 0) {
				return EINVAL;
			}
			set_inherit = 1;
		}

		/*
		 * set_runmask() currently does the above
		 * check on the mask proper.
		 */
		if (new[0] != 0)
			set_rmask = 1;
#endif
		WR_VERIFY_PTR(act, kap->data,
		    sizeof(struct _thread_runmask) + 2 * num * sizeof(unsigned));
		WR_PROBE_INT(act, kap->data,
		    sizeof(struct _thread_runmask) / sizeof(int) + 2 * num);

		lock_kernel();
		tr->size = num;
		uintp = (unsigned *)(tr + 1);
#if RMSK_MAX > 1
		/*
		 * NYI
		 * - memcpy() out the current masks
		 * - set_rumask() needs to handle larger sets.
		 */
#else
		*uintp = ~op->runmask & LEGAL_CPU_BITMASK;
		uintp++;
		*uintp = ~op->default_runmask & LEGAL_CPU_BITMASK;
		if(set_rmask != 0 && set_runmask(op, new[0]) != 0) {
			return ENOERROR;
		}
		if (set_inherit)
			op->default_runmask = ~inherit[0];
#endif
		break;


	case _NTO_TCTL_RUNMASK_GET_AND_SET:
		unlock_kernel();
		WR_VERIFY_PTR(act, kap->data, sizeof(unsigned));
		WR_PROBE_INT(act, kap->data, 1);
		new[0] = *(unsigned *)kap->data;
		lock_kernel();
		*(unsigned *)kap->data = ~op->runmask & LEGAL_CPU_BITMASK;
		if(new[0] != 0 && set_runmask(op, new[0]) != 0) {
			return ENOERROR;
		}
		break;
	case _NTO_TCTL_ALIGN_FAULT:
		unlock_kernel();
		WR_VERIFY_PTR(act, kap->data, sizeof(int));
		WR_PROBE_INT(act, kap->data, 1);
		prev = (op->flags & _NTO_TF_ALIGN_FAULT) ? 1 : -1;
		lock_kernel();
		if(*(int *)kap->data > 0) {
			op->flags |= _NTO_TF_ALIGN_FAULT;
		} else if( *(int *)kap->data < 0 ) {
			op->flags &= ~_NTO_TF_ALIGN_FAULT;
		}
		cpu_thread_align_fault(op);
		*(int *)kap->data = prev;
		break;

	case _NTO_TCTL_NAME:
		unlock_kernel();

		RD_VERIFY_PTR(act, kap->data, sizeof(struct _thread_name));
		RD_PROBE_INT(act, kap->data, sizeof(struct _thread_name) / sizeof(int));
		tn = (struct _thread_name *)kap->data;
		if(tn->new_name_len > _NTO_THREAD_NAME_MAX) {
			return E2BIG;
		}

		if(tn->name_buf_len < 0 || tn->new_name_len > tn->name_buf_len) {
			return EINVAL;
		}

		if(tn->new_name_len > 0) {
			RD_VERIFY_PTR(act, tn->name_buf, tn->new_name_len);
			RD_PROBE_INT(act, tn->name_buf, (tn->new_name_len + (sizeof(int) - 1)) / sizeof(int));
		}
		if(tn->name_buf_len > 0) {
			WR_VERIFY_PTR(act, tn->name_buf, tn->name_buf_len);
			WR_PROBE_INT(act, tn->name_buf, (tn->name_buf_len + (sizeof(int) - 1)) / sizeof(int));
		}
		lock_kernel();

		prev_name = op->name;
		prev_name_len = (op->name) ? strlen(op->name) + 1 : 0;

		//Setting a new name for the thread
		//NOTE: Due to the nature of the incoming/outgoing names we always re-allocate
		//a buffer here since otherwise we would have to allocate a temporary buffer
		//in order to be able to return the previous name to the user.  TODO: Investigate
		//a character by character copy to avoid allocations.
		if(tn->new_name_len >= 0) {
			if(tn->new_name_len >= THREAD_NAME_FIXED_SIZE) {
				op->name = _smalloc(tn->new_name_len + 1);	//Ensure a null
			} else if (tn->new_name_len > 0) {
				op->name = object_alloc(op->process, &threadname_souls);
			} else {
				op->name = NULL;
			}

			if(op->name) {
				memcpy(op->name, tn->name_buf, tn->new_name_len);
				op->name[tn->new_name_len] = '\0';		//Force a null
			}

			_TRACE_TH_EMIT_NAME(op);
		}

		//Return the previous name (if setting) or the current name
		if(tn->name_buf_len > 0) {
			if(prev_name_len > tn->name_buf_len) {
				memcpy(tn->name_buf, prev_name, tn->name_buf_len - 1);
				tn->name_buf[tn->name_buf_len - 1] = '\0';
			} else if(prev_name_len) {
				memcpy(tn->name_buf, prev_name, prev_name_len);
			} else {
				tn->name_buf[0] = '\0';						//No name, return null string
			}
		}

		//Deallocate previous name if we changed it
		if(prev_name_len && prev_name != op->name) {
			if(prev_name_len >= THREAD_NAME_FIXED_SIZE) {
				_sfree(prev_name, prev_name_len);
			} else {
				object_free(op->process, &threadname_souls, prev_name);
			}
			prev_name = NULL;
		}
		break;

	case _NTO_TCTL_PERFCOUNT:
	default:
		return EINVAL;
	}

	return EOK;
}

__SRCVERSION("ker_thread.c $Rev: 204471 $");
