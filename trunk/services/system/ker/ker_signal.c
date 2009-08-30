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
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
 */

#include "externs.h"
#include "mt_kertrace.h"

int
do_ker_signal_kill(THREAD *act, struct kerargs_signal_kill *kap) {
	PROCESS		*prp;
	THREAD		*thp;
	int			status;
	pid_t		src_pid;
	pid_t		tgt_pid;
	unsigned	flags;
	unsigned	signo;

	/* split out so the network signal kill can use this code. */

	signo = kap->signo;
	if(signo & SIG_TERMER_NOINHERIT) {
		signo &= ~SIG_TERMER_NOINHERIT;
		flags = 0;
	} else {
		flags = SIGNAL_KILL_TERMER_INHERIT_FLAG;
	}

	// Verify the signal and code are valid
	if((signo > _SIGMAX) || (kap->code > SI_USER)) {
		return EINVAL;
	}

	// A pid of zero for kill in POSIX means my process group.
	tgt_pid = kap->pid;
	if(tgt_pid == 0) tgt_pid = -act->process->pgrp;

	// We have three cases to consider
	// pid  tid  Action
	// < 0  ---  Hit a process group.
	// >=0  = 0  Hit a single process.
	// >=0  > 0  Hit a single thread.

	if(tgt_pid == -1) {
		// We don't allow people to try and kill procnto's process group
		return ESRCH;
	}
	if(tgt_pid > 0) {
		/*
		 * Some sanity test when hitting a process.  If
		 * hitting a group, these are done as the group
		 * members are found.
		 */

		// Verify the target process exists
		prp = lookup_pid(tgt_pid);
		if(prp == NULL) {
			return ESRCH;
		}

		// Verify we have the right to hit the target process
		if(!keriskill(act, prp, signo)) {
			return ENOERROR;
		}

		// If specified, verify the target thread exists
		thp = NULL;
		if(kap->tid != 0 && (thp = vector_lookup(&prp->threads, kap->tid-1)) == NULL) {
			return ESRCH;
		}

		// A signo of zero just queries existance.
		if(signo == 0) {
			return EOK;
		}

		lock_kernel();

		src_pid = act->process->pid;
		// Since this is a user request for a signal and can handle
		// an error return, we shouldn't delve into the critical list
		// for any pulse that we need to allocate for the signal - save
		// the critical heap for important pulses like hwintrs and
		// CPU exception fault delivery
		pulse_souls.flags &= ~SOUL_CRITICAL;
		if(kap->tid == 0) {
			status = signal_kill_process(prp, signo, kap->code,
						kap->value, src_pid, flags);
		} else {
			status = signal_kill_thread(prp, thp, signo, kap->code,
						kap->value, src_pid, flags);
		}
		pulse_souls.flags |= SOUL_CRITICAL;
	} else {
		lock_kernel();

		src_pid = act->process->pid;
		// Since this is a user request for a signal and can handle
		// an error return, we shouldn't delve into the critical list
		// for any pulse that we need to allocate for the signal - save
		// the critical heap for important pulses like hwintrs and
		// CPU exception fault delivery
		pulse_souls.flags &= ~SOUL_CRITICAL;
		status = signal_kill_group(act, -tgt_pid, signo, kap->code,
				kap->value, src_pid, flags);
		pulse_souls.flags |= SOUL_CRITICAL;
	}

	if(status == 0) {
		kererr(act, EPERM);
	} else if(status & SIGSTAT_ESRCH) {
		kererr(act, ESRCH);
	} else if(status & SIGSTAT_NOTQUEUED) {
		kererr(act, EAGAIN);
	} else {
		SETKSTATUS(act, 0);
	}
	return ENOERROR;
}

//
// Set a signal on a process group, process or thread.
//
int kdecl
ker_signal_kill(THREAD *act, struct kerargs_signal_kill *kap) {

	// Check if nd is local
	if(kap->nd != ND_LOCAL_NODE) {
		return net_deliver_signal(act, kap);
	}
	return do_ker_signal_kill(act, kap);
}


//
// This kernel call is called by the signal stub to return from a signal
// handler. It is very unlikely that an application would ever call it
// directly.
// When a signal is delivered the path is as follows.
//
// kernel call sigstub
//     sigstub calls user handler
//         user handler runs and returns to sigstub
//     sigstub invokes SignalReturn kernel call
// we arrive here in the kernel
//
int kdecl
ker_signal_return(THREAD *act, struct kerargs_signal_return *kap) {
	SIGSTACK		*ssp = kap->s;
	ucontext_t		*uc;
	TIMER			*tip;
	uintptr_t		old_ip;


	// Make sure the data is there...
	RD_VERIFY_PTR(act, ssp, sizeof(*ssp));
	RD_PROBE_INT(act, ssp, sizeof(*ssp) / sizeof(int));

	// Restore registers which were damaged. Pull out one before locking
	// the kernel to make sure the 'uc' pointer is good.
	uc = ssp->info.context;
	old_ip = REGIP(&uc->uc_mcontext.cpu);
	lock_kernel();
	cpu_signal_restore(act, ssp);
	SETKIP(act, old_ip);
	SETKSP(act, REGSP(&uc->uc_mcontext.cpu));

	// If the signal interrupted while a TimerTimeout was in effect
	// we restore the timeout timer.
	if(ssp->timeout_flags  &&  (tip = act->timeout)) {
		act->timeout_flags = ssp->timeout_flags;
		tip->itime.nsec = ssp->timeout_time;
		tip->event = ssp->timeout_event;
	}
#ifdef _mt_LTT_TRACES_	/* PDB */
//	if (tip == NULL)
//		mt_TRACE_DEBUG("tip is null");
//	else
		//mt_trace_task_sig_return(tip->thread->process->pid, tip->thread->tid);
	mt_trace_task_sig_return(act->process->pid, act->tid);
#endif

	// If the signal interrupted while waiting for a mutex then we set
	// a flag to attempt to reaquire it in specialret() before returning.
	if(ssp->mutex) {
		act->flags |= _NTO_TF_ACQUIRE_MUTEX;
		act->args.mu.mutex = ssp->mutex;
		act->args.mu.saved_timeout_flags = ssp->mutex_timeout_flags;
		act->args.mu.incr = ssp->mutex_acquire_incr;

		// If we were waiting for a mutex with an active timeout,
		// have to re-establish the timeout.
		if(act->args.mu.saved_timeout_flags & _NTO_TIMEOUT_ACTIVE) {
			// Turn off the active bit, since the timeout isn't actually
			// active yet and having the bit on will prevent timeout_start()
			// from doing anything.
			act->timeout_flags = act->args.mu.saved_timeout_flags
									& ~_NTO_TIMEOUT_ACTIVE;

// 			You'd think we'd have to activate the timeout timer here, but
//			we're not actually actually blocked at this point. When the
//			_NTO_TF_ACQUIRE_MUTEX code runs in specret it will call block()
//			(if needed) and that will activate the timer.
//			timer_activate(act->timeout);
		}
	}

	// Restore the "error set" flag if it was on originally.
	act->flags |= ssp->old_flags & _NTO_TF_KERERR_SET;

	// Restore saved signal blocked mask.
	signal_block(act, &ssp->sig_blocked);
	return ENOERROR;
}


//
// Setup a signal handler for a process.
//
int kdecl
ker_signal_action(THREAD *act, struct kerargs_signal_action *kap) {
	PROCESS				*prp;
	SIGHANDLER			*shp;
	struct sigaction	*oact, new;

	// Verify the signal is valid
	if(kap->signo <= 0  ||  kap->signo > SIGRTMAX) {
		return EINVAL;
	}

	// Verify the target process exists
	prp = kap->pid ? lookup_pid(kap->pid) : act->process;
	if(prp == NULL) {
		return ESRCH;
	}

	// Verify we have the right to examine the process
	if(!kerisusr(act, prp))
		return ENOERROR;

	// Check if a new action has been specified and make sure it is allowed.
	if(kap->act) {
		RD_VERIFY_PTR(act, kap->act, sizeof(*kap->act));
		// Make local copy incase kap->act==kap->oact since we change oact.
		new = *kap->act;

		// Set/Clear the queued bit based upon the SA_SIGINFO and use sigaction
		if(new.sa_flags & SA_SIGINFO) {
			new.sa_handler = new.sa_sigaction;
		}

		// SA_RESTART and SA_ONSTACK are not currently supported
		if(new.sa_flags & (SA_MASK &
				~(	SA_SIGINFO |
					SA_NOCLDSTOP |
					SA_NOCLDWAIT |
					SA_RESETHAND |
					SA_NODEFER))) {
			return ENOTSUP;
		}

		// You can only set SIGKILL and SIGSTOP to their default action.
		if(new.sa_handler != SIG_DFL && (kap->signo == SIGKILL || kap->signo == SIGSTOP)) {
			return EINVAL;
		}
	} else {
		memset(&new, 0, sizeof(new));
	}

	// Must lock before get_signal_handler(), as it may do an object_alloc
	if((oact = kap->oact)) {
		WR_VERIFY_PTR(act, oact, sizeof(*oact));
		WR_PROBE_INT(act, &oact->sa_mask, sizeof(oact->sa_mask) / sizeof(int));
		WR_PROBE_INT(act, &oact->sa_handler, sizeof(oact->sa_handler) / sizeof(int));
	}
	lock_kernel();

	// We reserve the first 15 values for special actions. Above that and its
	// the address of a signal handler. The last arg means "create an entry"
	// if one does not already exist.
	shp = get_signal_handler(prp, kap->signo,
				kap->act && (((uintptr_t)new.sa_handler > 15) ||
				(new.sa_flags & (SA_MASK & ~(SA_SIGINFO | SA_NOCLDSTOP)))));

	// If requested save current signal state
	if(oact) {
		if(shp) {
			// We have a signal handler
			oact->sa_mask    = shp->sig_blocked;
			oact->sa_handler = shp->handler;
			oact->sa_flags   = shp->flags;
		} else {
			// No handler so assume default
			SIGMASK_ZERO(&oact->sa_mask);
			oact->sa_handler = SIG_DFL;
			oact->sa_flags   = 0;
		}

		// Check if signal is ignored (overrides what was done just above)
		if(SIG_TST(prp->sig_ignore, kap->signo)) {
			oact->sa_handler = SIG_IGN;
		}

		// POSIX defines two fields for the address of the signal handler
		oact->sa_sigaction = oact->sa_handler;

		// We keep the NOCLDSTOP indication in the process flags and
		// the queued flags in process sig_queue.
		if(kap->signo == SIGCHLD) {
			if(prp->flags & _NTO_PF_NOCLDSTOP) {
				oact->sa_flags |= SA_NOCLDSTOP;
			}
			if(SIG_TST(prp->sig_ignore, kap->signo)) {
				oact->sa_flags |= SA_NOCLDWAIT;
				// @@@ Should remember original sa_handler
			}
		}
		if(SIG_TST(prp->sig_queue, kap->signo)) {
			oact->sa_flags |= SA_SIGINFO;
		}
	}

	// If requested set new signal state
	if(kap->act) {

		// Set/Clear the queued bit based upon the SA_SIGINFO and use sigaction
		if(new.sa_flags & SA_SIGINFO) {
			SIG_SET(prp->sig_queue, kap->signo);
		} else {
			SIG_CLR(prp->sig_queue, kap->signo);
		}

		// We keep NOCLDSTOP indication in the process flags.
		if(kap->signo == SIGCHLD) {
			if(new.sa_flags & SA_NOCLDSTOP) {
				prp->flags |= _NTO_PF_NOCLDSTOP;
			} else {
				prp->flags &= ~_NTO_PF_NOCLDSTOP;
			}
			if(new.sa_flags & SA_NOCLDWAIT) {
				// @@@ Should remember original sa_handler
				new.sa_handler = SIG_IGN;
			}
		}

		// Remember flags ignoring ones that will be stuffed from other information
		if(shp) {
			shp->handler = NULL;
			shp->flags = new.sa_flags & ~(SA_NOCLDWAIT | SA_NOCLDSTOP | SA_SIGINFO);
		}

		// Switch in signal action.
		switch((intptr_t)new.sa_handler) {
		case (intptr_t)SIG_IGN:
			// You cannot ignore these signals.
			if(kap->signo != SIGKILL  &&  kap->signo != SIGSTOP) {
				signal_ignore(prp, kap->signo);
			}
			break;

		case (intptr_t)SIG_DFL:
			// Remove pending signals whose default action is to ignore
			// SIGCHLD/SIGURG are specified by POSIX.
			// The others are here to match special_case_signalhandling()
			if (kap->signo == SIGCHLD || kap->signo == SIGURG ||
				kap->signo == SIGCONT ||
			    kap->signo == SIGPWR || kap->signo == SIGWINCH) {
				signal_ignore(prp, kap->signo);
			}
			SIG_CLR(prp->sig_ignore, kap->signo);
			break;

#if 0	// This is for when we handle SIG_HOLD
		case (intptr_t)SIG_HOLD:
			SIG_CLR(prp->sig_ignore, kap->signo);
			if(shp == NULL) {
				return EAGAIN;
			}
			shp->flags |= INTERNAL_SA_HOLD;
			break;
#endif
		default:
			// We reserve the first 15 values for special actions.
			if((uintptr_t)new.sa_handler <= 15) {
				return EINVAL;
			}
			// The default case installs a handler which removes any ignore
			SIG_CLR(prp->sig_ignore, kap->signo);
			if(shp == NULL) {
				return EAGAIN;
			}

			// Save signal stub which we call into, user handler and mask.
			if(kap->sigstub) {
				prp->sigstub = kap->sigstub;
			}
			shp->handler = new.sa_handler;
			SIGMASK_CPY(&shp->sig_blocked, &new.sa_mask);
			SIGMASK_NO_KILLSTOP(&shp->sig_blocked);
			break;
		}
	}

	return EOK;
}


//
// Change the signal block mask for a thread.
// This call is slightly overloaded to also handle sigpending().
//
int kdecl
ker_signal_procmask(THREAD *act, struct kerargs_signal_procmask *kap) {
	sigset_t newbits, sig_blocked;
	THREAD	*thp;
	PROCESS *prp;

	// Verify the target process exists
	prp = kap->pid ? lookup_pid(kap->pid) : act->process;
	if(prp == NULL) {
		return ESRCH;
	}

	// Verify we have the right to modify the process
	if(!kerisusr(act, prp))
		return ESRCH;

	// If specified, verify the target thread exists
	thp = kap->tid ? vector_lookup(&prp->threads, kap->tid-1) : act;
	if(thp == NULL) {
		return ESRCH;
	}

	// Make local copy incase kap->sig_blocked==kap->old_sig_blocked since
	// we change old_sig_blocked first.
	if(kap->sig_blocked) {
		RD_VERIFY_PTR(act, kap->sig_blocked, sizeof(*kap->sig_blocked));
		SIGMASK_CPY(&sig_blocked, kap->sig_blocked);
	} else {
		memset(&sig_blocked, 0, sizeof(sig_blocked));
	}

	if(kap->old_sig_blocked) {
		WR_VERIFY_PTR(act, kap->old_sig_blocked, sizeof(*kap->old_sig_blocked));
		WR_PROBE_OPT(act, kap->old_sig_blocked, sizeof(*kap->old_sig_blocked) / sizeof(int));
	}

	// We slipped in code to handle sigpending() to save common code above
	if(kap->how == SIG_PENDING) {
		PULSE *pup;

		// Get all the pending bits. We must scan the queue.
		SIGMASK_ZERO(&newbits);
		for(pup = pril_first(&thp->sig_pending); pup ; pup = pril_next(pup)) {
			unsigned	n;

			if(TYPE_MASK(pup->type) == TYPE_SIGNAL) { // only TYPE_SIGNAL
				n = PRI_TO_SIG(pup->priority);
				SIG_SET(newbits, n);
			}
		}
		for(pup = pril_first(&thp->process->sig_pending); pup ; pup = pril_next(pup)) {
			unsigned	n;

			if(TYPE_MASK(pup->type) == TYPE_SIGNAL) { // only TYPE_SIGNAL
				n = PRI_TO_SIG(pup->priority);
				SIG_SET(newbits, n);
			}
		}

		kap->old_sig_blocked->__bits[0] = newbits.__bits[0] & thp->sig_blocked.__bits[0];
		kap->old_sig_blocked->__bits[1] = newbits.__bits[1] & thp->sig_blocked.__bits[1];

		lock_kernel();

		return EOK;
	}

	if(kap->old_sig_blocked) {
		SIGMASK_CPY(kap->old_sig_blocked, &thp->sig_blocked);
	}

	// Set the blocked mask as requested
	if(kap->sig_blocked) {
		SIGMASK_CPY(&newbits, &thp->sig_blocked);

		switch(kap->how) {
		case SIG_BLOCK:
			SIGMASK_SET(&newbits, &sig_blocked);
			break;

		case SIG_UNBLOCK:
			SIGMASK_CLR(&newbits, &sig_blocked);
			break;

		case SIG_SETMASK:
			SIGMASK_CPY(&newbits, &sig_blocked);
			break;

		default:
			return EINVAL;
		}

		lock_kernel();

		// The top 8 signal bits (of 64) may not be changed.
		SIGMASK_SPECIAL(&newbits);
		// Do the work. This may cause pending signals to be acted upon
		signal_block(thp, &newbits);
	}

	lock_kernel();

	return EOK;
}


//
// The POSIX sigsuspend() call. Better to use sigwaitinfo().
//
int kdecl
ker_signal_suspend(THREAD *act, struct kerargs_signal_suspend *kap) {
	sigset_t bits;
	unsigned	tls_flags;

	// Get a local copy to prevent a fault after we lock the kernel.
	RD_VERIFY_PTR(act, kap->sig_blocked, sizeof(*kap->sig_blocked));
	bits = *kap->sig_blocked;
	tls_flags = act->un.lcl.tls->__flags;
	lock_kernel();

	// Save current mask and install new mask.
	SIGMASK_CPY(&act->args.ss.sig_blocked, &act->sig_blocked);
	SIGMASK_SPECIAL(&bits);
	signal_block(act, &bits);

	// Are there any unblocked signals queued on this process.
	if((act->flags & _NTO_TF_SIG_ACTIVE) == 0) {
		// No signal queued so we will attempt to block.

		// Check for an immediate timeout.
		if(IMTO(act, STATE_SIGSUSPEND)) {
			SIGMASK_CPY(&act->sig_blocked, &act->args.ss.sig_blocked);
			return ETIMEDOUT;
		}

		// Check for a pending cancelation
		if(PENDCAN(tls_flags)) {
			SETKIP_FUNC(act, act->process->canstub);
			return ENOERROR;
		}

		// Block
		act->state = STATE_SIGSUSPEND;
		_TRACE_TH_EMIT_STATE(act, SIGSUSPEND);
		block();
		act->flags |= _NTO_TF_SIGSUSPEND;
		// Try to optimize signal delivery by looking at this thread first.
		act->process->sigtid_cache = act->tid;
	} else {
		// A signal is queued so force immediate error return.
		// Mask will be restored in specialret().
		kererr(act, EINTR);
		act->flags |= _NTO_TF_SIGSUSPEND;
	}
	return ENOERROR;
}


//
// Block waiting for a signal. A set of signals to wait for are specified.
// The waited for signals are typically blocked before this call. A signal
// handler will not be entered.
//
int kdecl
ker_signal_waitinfo(THREAD *act, struct kerargs_signal_wait *kap) {
	PULSE			*pup;
	siginfo_t		*sip;
	sigset_t		 sig_wait;
	int				 signo, i;
	unsigned		 tls_flags;
	PRIL_HEAD		*ph;

	RD_VERIFY_PTR(act, kap->sig_wait, sizeof(*kap->sig_wait));
	sip = kap->sig_info;
	if(sip!=NULL)	 {
		if (!WITHIN_BOUNDRY((uintptr_t)sip, (uintptr_t)sip + sizeof(*sip), act->process->boundry_addr)) {
			return EFAULT;
		}
		WR_VERIFY_PTR(act, sip, sizeof(*sip));
	}
	sig_wait = *kap->sig_wait;

	// Check for a pending signal.
	// We first check the thread than the process.
	ph = &act->sig_pending;
	for(i = 0; i < 2; ++i) {
		for(pup = pril_first(ph); pup != NULL ; pup = pril_next(pup)) {
			// Check if a pending signal matches the sig_wait mask.
			signo = PRI_TO_SIG(pup->priority);
			if(SIG_TST(sig_wait, signo)) {
				if(sip) {	// May be past as NULL
					WR_PROBE_OPT(act, sip, sizeof(*sip) / sizeof(int));
					sip->si_signo = signo;
					sip->si_code = pup->code;
					sip->si_value.sival_int = pup->value;
					sip->si_pid = pup->id;
				}

				lock_kernel();

				// If debugger stops the process, don't deliver the signal
				if(act->process->debugger  &&  (*debug_thread_signal)(act, signo, pup->code, pup->value, pup->id)) {
					KIP(act) -= KER_ENTRY_SIZE;
					return ENOERROR;
				}

				// Cancel any timeout and return requested info to application.
				act->timeout_flags = 0;
				SETKSTATUS(act,signo);

				// Remove the signal
				pulse_remove(act->process, ph, pup);

				return ENOERROR;
			}
		}

		// Now check the process.
		ph = &act->process->sig_pending;
	}

    tls_flags = act->un.lcl.tls->__flags;
	lock_kernel();
	// Check for an immediate timeout.
	if(IMTO(act, STATE_SIGWAITINFO)) {
		return ETIMEDOUT;
	}

	// Check for a pending cancellation
	if(PENDCAN(tls_flags)) {
		SETKIP_FUNC(act,act->process->canstub);
		return ENOERROR;
	}

	// Block the thread.
	act->state = STATE_SIGWAITINFO;
	_TRACE_TH_EMIT_STATE(act, SIGWAITINFO);
	block();
	// Try to optimize signal delivery by looking at this thread first.
	act->process->sigtid_cache = act->tid;

	// Save the sigwait blocked mask and a pointer to user sig_info buffer.
	SIGMASK_CPY(&act->args.sw.sig_wait, &sig_wait);
	SIGMASK_NO_KILLSTOP(&act->args.sw.sig_wait);
	act->args.sw.sig_info = sip;
	return ENOERROR;
}

//
// This will cause the calling thread to act like it faulted with the
// calling register set.
// It is used by the floating point emulation code.
//
int kdecl
ker_signal_fault(THREAD *act, struct kerargs_signal_fault *kap) {
	unsigned			sigcode = kap->sigcode;
	uintptr_t			addr = kap->addr;
	unsigned			flags;

	RD_VERIFY_PTR(act, kap->regs, sizeof(CPU_REGISTERS));
	RD_PROBE_INT(act, kap->regs, sizeof(CPU_REGISTERS) / sizeof(int));

	lock_kernel();

	flags = act->internal_flags;
	act->internal_flags &= ~_NTO_ITF_SSTEP_SUSPEND;

	sigcode &= ~SIGCODE_SSTEP;

	cpu_greg_load(act, kap->regs);
	if(sigcode) {
		usr_fault(sigcode, act, addr);
	} else if(flags & _NTO_ITF_SSTEP_SUSPEND) {
		// We've finished emulating a F.P. instruction. If we were
		// single stepping, we want to stop right here.
		usr_fault(MAKE_SIGCODE(SIGTRAP,TRAP_TRACE,FLTTRACE), act, addr);
	} else {
		// Make setting this bit stops other code from changeing the IP
		act->flags |= _NTO_TF_KERERR_SET;
	}
	return ENOERROR;
}

__SRCVERSION("ker_signal.c $Rev: 204340 $");
