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

#define FLTNO_BDSLOT_INDICATOR	0x80000000


//
// Discard all queued signals with a given number. This is called
// whenever the ignored mask is changed.
//
// Note that when a signal is enqueued as a pulse we map the signo
// into a priority.
//
static void
signal_discard(PRIL_HEAD *queue, PROCESS *prp, int signo) {
	PULSE	*pup;
	PULSE	*next;
	int 	priority = SIG_TO_PRI(signo);

	for(pup = pril_find_insertion(queue, priority | _PULSE_PRIO_HEAD);
			(pup != NULL) && (pup->priority == priority);
			pup = next) {
		next = pril_next(pup);
		// Remove signal object.
		pril_rem(queue, pup);
		object_free(prp, &pulse_souls, pup);
	}
}


// remove signal from either process or thread pending signals
static void
signal_remove(PROCESS *prp, int signo) {
	int 	i;
	THREAD	*thp;
	PULSE	*pup;

	signal_discard(&prp->sig_pending, prp, signo);

	// Remove thread pending signals and clear signal active flags if no
	// signals are now pending and unblocked.
	for(i = 0 ; i < prp->threads.nentries ; ++i) {
		if((unsigned)i < prp->threads.nentries && (thp = VECP(thp, &prp->threads, i))) {
			signal_discard(&thp->sig_pending, prp, signo);

			// Check for if any active signals are remaining....
			pup = pril_first(&thp->sig_pending);
			for( ;; ) {
				if(pup == NULL) {
					// Got to the end of the list with no active signals...
					thp->flags &= ~_NTO_TF_SIG_ACTIVE;
					break;
				}
				if(!SIG_TST(thp->sig_blocked, PRI_TO_SIG(pup->priority))) break;
				pup = pril_next(pup);
			}
		}
	}
}

// this function handles special case-ing for specific signals.
// returns -1 if no special case action is to be taken
static int
special_case_signalhandling(PROCESS *prp, THREAD *thp, int signo, int code, int value, pid_t pid) {
	SIGHANDLER *shp;

	// Check for special action signals.
	if(signo < SIGRTMIN) {
		switch(signo) {
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			if((shp = get_signal_handler(prp, signo, 0)) && shp->handler) {
				// Remove any pending SIGCONT
				signal_remove(prp, SIGCONT);
				return(-1);
			}
			// Fall Through
		case SIGSTOP:
			// don't stop if process is being destroyed
			if(prp->flags & _NTO_PF_DESTROYALL) {
				return SIGSTAT_QUEUED;
			}

			// SIGSTOP and SIGCONT cannot be blocked and perform special actions.
			// Tell the process manager we stopped
			if(procmgr.process_stop_or_cont) {
				struct sigevent	ev;

				if(procmgr.process_stop_or_cont(prp, signo, code, value, pid, &ev)) {
					sigevent_proc(&ev);
				}
			}

			// Tell the debugger we stopped
			if(prp->debugger) {
				(void)(*debug_process_stopped)(prp, signo, code, value, pid);
			}

			// Drop a SIGCHLD on the parent
			if((prp->flags & _NTO_PF_STOPPED) == 0) {
				// Fill out report to parent
				prp->siginfo.si_signo = SIGCHLD;
				prp->siginfo.si_code = CLD_STOPPED;
				prp->siginfo.si_errno = 0;
				prp->siginfo.si_status = signo;
				prp->siginfo.si_pid = prp->pid;

				if((prp->parent->flags & _NTO_PF_NOCLDSTOP) == 0) {
					signal_kill_process(prp->parent, SIGCHLD, CLD_STOPPED, signo, prp->pid, 0);
				}

				prp->flags |= _NTO_PF_STOPPED;
				prp->flags &= ~_NTO_PF_CONTINUED;
				stop_threads(prp, 0, 0);
			}

			// Remove any pending SIGCONT
			signal_remove(prp, SIGCONT);

			return(SIGSTAT_QUEUED);

		case SIGCONT:
			// Check if being debugged
			// If debugger stops the process, don't deliver the signal
			if(thp && prp->debugger && (*debug_thread_signal)(thp, signo, code, value, pid)) {
				return(SIGSTAT_QUEUED);
			}

			// Tell the process manager we continued
			if(procmgr.process_stop_or_cont) {
				struct sigevent	ev;

				if(procmgr.process_stop_or_cont(prp, signo, code, value, pid, &ev)) {
					sigevent_proc(&ev);
				}
			}

			if((prp->flags & _NTO_PF_CONTINUED) == 0) {
				// Fill out report to parent
				prp->siginfo.si_signo = SIGCHLD;
				prp->siginfo.si_code = CLD_CONTINUED;
				prp->siginfo.si_errno = 0;
				prp->siginfo.si_status = signo;
				prp->siginfo.si_pid = prp->pid;
			}
			prp->flags &= ~_NTO_PF_STOPPED;
			prp->flags |= _NTO_PF_CONTINUED;
			cont_threads(prp, 0);

			// Remove any pending stop signals
			signal_remove(prp, SIGSTOP);
			signal_remove(prp, SIGTSTP);
			signal_remove(prp, SIGTTIN);
			signal_remove(prp, SIGTTOU);

			// Check if process has a SIGCONT handler
			if((shp = get_signal_handler(prp, signo, 0)) && shp->handler) {
				return(-1);
			}
			return(SIGSTAT_QUEUED);

		/*
		 * SIGIO is a BSD signal that is ignored, but it shadows SIGPOLL
		 * which unix98 states should not be ignored.
		 */
		case SIGPWR:
		case SIGURG:
		case SIGWINCH:
		case SIGCHLD:
			// Default action is to ignore. We can't use ignore bit because
			// we overload it to mean discard exit status.
			shp = get_signal_handler(prp, signo, 0);
			if(shp == NULL  ||  shp->handler == NULL) {
				return(SIGSTAT_QUEUED);
			}
			break;

		case SIGKILL:
			if(prp->flags & _NTO_PF_STOPPED) {
				// This signal always kills you so make sure we continue any
				// threads first.
				prp->flags &= ~_NTO_PF_STOPPED;
				cont_threads(prp, 0);
			}
			if(thp->flags & _NTO_TF_THREADS_HOLD) {
				// We are on hold and this signal always kills you so make
				// sure we continue any threads first.
				cont_threads(prp, _NTO_TF_THREADS_HOLD);
			}
			prp->flags |= _NTO_PF_DESTROYALL;
			break;

		default:
			break;
		}
	}
	return(-1);
}

//
// Set a signal on a thread.
//
// Returns
//   SIGSTAT_NOPERMS
//   SIGSTAT_IGNORED
//   SIGSTAT_QUEUED
//
int rdecl
signal_kill_thread(PROCESS *prp, THREAD *thp, int signo, int code, int value,
					pid_t pid, unsigned signal_flags) {
// signal_flags is an or-ing of kermacros.h:: SIGNAL_KILL_*_FLAG
	PULSE		*pup;
	int 		status;
	unsigned	priority;

	// Check against ignore mask and discard if set
	// Ignoring SIGCONT is equivalent to default action
	if(SIG_TST(prp->sig_ignore, signo) && signo != SIGCONT) {
		return(SIGSTAT_IGNORED);
	}
	if(signal_flags & SIGNAL_KILL_TERMER_INHERIT_FLAG) {
		// want to inherit terming prio from killer (most SignalKill() calls)
		prp->terming_priority = actives[KERNCPU]->priority;
	} else if ( signal_kill_updates_terming_priority ) {
		// use receiving thread for prio (interrupt or SIG_TERMER_NOINHERIT flag)
		prp->terming_priority = thp->priority;
	}

	// First check if the thread is in a sigwaitinfo(..)
	if(thp->state == STATE_SIGWAITINFO  &&  SIG_TST(thp->args.sw.sig_wait, signo)) {
		// If debugger stops the process, don't deliver the signal
		if(prp->debugger && (*debug_thread_signal)(thp, signo, code, value, pid))
			return(SIGSTAT_QUEUED);

		// Since we are not likely in the context of the target thread we
		// save the info in the target threads table and set a flag which
		// tells the kernel to copy it when we next run the thread.
		thp->args.sw.signo = signo;
		thp->args.sw.code = code;
		thp->args.sw.value = value;
		thp->args.sw.pid = pid;
		thp->flags |= _NTO_TF_SIGWAITINFO;
		if(signal_flags & SIGNAL_KILL_APS_CRITICAL_FLAG) AP_MARK_THREAD_CRITICAL(thp);
		ready(thp);
		return(SIGSTAT_QUEUED);
	}

	status = special_case_signalhandling(prp, thp, signo, code, value, pid);
	if(status != -1) {
		return(status);
	}

	// Add the signal. If signals are not queued replace. Never queue SI_TIMER.
	// note: at this point we lose memory that we might have to deliver this
	// pulse as critical for the APS scheduler PR27529
	priority = _PULSE_PRIO_SIGNAL | SIG_TO_PRI(signo);
	if(!SIG_TST(prp->sig_queue, signo) || code == SI_TIMER) {
		priority |= _PULSE_PRIO_REPLACE;
	}
	pup = pulse_add(prp, &thp->sig_pending, priority, code, value, pid);

	// If the signal was not blocked we must ready the thread.
	if(SIG_TST(thp->sig_blocked, signo) == 0) {
		if(thp->state == STATE_MUTEX) {
			thp->flags |= _NTO_TF_ACQUIRE_MUTEX;
			thp->args.mu.saved_timeout_flags = thp->timeout_flags;
		}
		if(signal_flags & SIGNAL_KILL_APS_CRITICAL_FLAG) AP_MARK_THREAD_CRITICAL(thp);
		force_ready(thp, EINTR);
		thp->flags |= _NTO_TF_SIG_ACTIVE;

		// Is the target thread running on another processor in an SMP system?
		if(thp->state == STATE_RUNNING  &&  thp != actives[KERNCPU]) {
			SENDIPI(thp->runcpu, IPI_RESCHED);
		}
	}

	return(pup ? SIGSTAT_QUEUED : SIGSTAT_NOTQUEUED);
}


//
// Set a signal on one thread in the process. The first thread which
// does not have the signal blocked will get it. Since many multi-threaded
// applications will dedicate a single thread to signals we cache the last
// thread which got the signal and check it first. The cache starts off as
// the first thread.
//
// Returns
//   SIGSTAT_NOPERMS
//   SIGSTAT_IGNORED
//   SIGSTAT_QUEUED
//
int rdecl
signal_kill_process(PROCESS *prp, int signo, int code, int value, pid_t pid,
					unsigned signal_flags) {
// signal_flags is an or-in of kermacros.h:: SIGNAL_KILL_*_FLAG
	THREAD		*thp, *target;
	int	 		tid;
	int			iters=0;
	int			sigsent=0;
	int			sigkillstatus=0;
	unsigned	priority;

	// Check against ignore mask and discard if set
	// Ignoring SIGCONT is equivalent to default action
	if(SIG_TST(prp->sig_ignore, signo) && signo != SIGCONT) {
		return(SIGSTAT_IGNORED);
	}

	// Do a scan to see if any thread has the signal unblocked.
	// We cache the last thread which accepted the signal.
	// for special signals that cannot be blocked, we start from
	// tid 0, since the cont_threads and stop_threads functions
	// scan from tid 0.
	if ((signo == SIGKILL) || (signo == SIGCONT) || (signo == SIGSTOP)) {
		tid = 0;
	} else {
		tid = prp->sigtid_cache;
	}
	target = 0;
	do {
		// Get a pointer to thread tid if it exists.
		if((unsigned)tid < prp->threads.nentries && (thp = VECP2(thp, &prp->threads, tid))) {
			if(thp->state == STATE_SIGWAITINFO  &&  SIG_TST(thp->args.sw.sig_wait, signo)) {
				// Thread is waiting for this signal via SignalWaitinfo
				target = thp;
				break;
			} else {
				if(SIG_TST(thp->sig_blocked, signo) == 0) {
					// Thread does not have signal blocked.
					target = thp;
					if(thp->state != STATE_REPLY) {
						// Try to stay away from a thread that may need an unblock pulse
						if (signo == SIGKILL) {
							sigsent=1;
							sigkillstatus = signal_kill_thread(prp, target, signo,
								code, value, pid, signal_flags);
						} else {
							break;
						}
					}
				}
			}
		}

		tid = ((tid+1) % prp->threads.nentries);  // increment tid modulo num entries
		iters++;                                // increment iterations through loop
	} while(iters <= prp->threads.nentries);	// Loop until we have scanned all entries

	if(target) {
		if(!sigsent) {
			prp->sigtid_cache = target->tid;
			return(signal_kill_thread(prp, target, signo, code, value, pid, signal_flags));
		} else {
			return(sigkillstatus);
		}
	}

	//
	// None of the threads would accept the signal so we set in on the process.
	//

	// We special case some signals since default action is to ignore
	if(signo == SIGCHLD || signo == SIGPWR || signo == SIGURG || signo == SIGWINCH) {
		SIGHANDLER *shp;
		// Default action is to ignore. We can't use ignore bit because
		// we overload it to mean discard exit status.
		shp = get_signal_handler(prp, signo, 0);
		if(shp == NULL  ||  shp->handler == NULL) {
			return(SIGSTAT_QUEUED);
		}
	}

	if (signo == SIGCONT) {
		// Ensure that SIGCONT always continues the process
		(void)special_case_signalhandling(prp, 0, signo, code, value, pid);

		// Only add the signal if it is not ignored
		if(SIG_TST(prp->sig_ignore, signo)) {
			return(SIGSTAT_IGNORED);
		}
	}

	// Add the signal. If signals are not queued replace. Never queue SI_TIMER.
	// note: at this point we lose memory that we might have to deliver this
	// pulse as critical for the APS scheduler PR27529
	priority = _PULSE_PRIO_SIGNAL | SIG_TO_PRI(signo);
	if(!SIG_TST(prp->sig_queue, signo) || code == SI_TIMER) {
		priority |= _PULSE_PRIO_REPLACE;
	}
	(void)pulse_add(prp, &prp->sig_pending, priority, code, value, pid);

	return(SIGSTAT_QUEUED);
}



//
// Set a signal on a process group. This will set it on each process
// which is a member of the group.
//
// Returns
//   SIGSTAT_NOPERMS
//   SIGSTAT_IGNORED
//   SIGSTAT_QUEUED
//   SIGSTAT_QUEUED | SIGSTAT_IGNORED
//
int rdecl
signal_kill_group(THREAD *thp, pid_t pgrp, int signo, int code, int value,
					pid_t pid, unsigned signal_flags) {
	PROCESS	*victim;
	pid_t	pid2;
	int		status = SIGSTAT_ESRCH;

	// Walk the process table looking for members of the process group.
	for(pid2 = 1 ; pid2 < process_vector.nentries ; ++pid2) {
		if(VECP(victim, &process_vector, pid2)) {
			if(victim->pgrp == pgrp) {
				status &= ~SIGSTAT_ESRCH;
			      	if (keriskill(thp, victim, signo)) {
					if (signo == 0) {
						/*
						 * Not actually ignored, just
						 * a flag so originator doesn't
						 * throw an error.  The presence
						 * of one member is enough so
						 * we can early out.
						 */
						status |= SIGSTAT_IGNORED;
						break;
					}
					status |= signal_kill_process(victim, signo, code, value,
							pid, signal_flags);
				}
			}
		}
	}

	return(status);
}


//
// We took an unfixable fault when trying to set up the stack frame
// for a signal handler. Nothing to do but toast the process.
//
static void
sigstack_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) {
	PULSE		*pup;
	int			 signo;
	SIGHANDLER	*shp;

	//kernel.s will have turned off the SIG_ACTIVE bit since it wants to
	//return EFAULT on the 'kernel call' we're doing, but we want to
	//come back into signal_specret() with no signal handler installed.

	thp->flags |= _NTO_TF_SIG_ACTIVE;
	signo = 0;
	for(pup = pril_first(&thp->sig_pending); pup; pup = pril_next(pup)) {
		signo = PRI_TO_SIG(pup->priority);
		if(SIG_TST(thp->sig_blocked, signo) == 0) break;
	}
	shp = get_signal_handler(thp->process, signo, 0);
	if(shp == NULL) crash();
	shp->handler = NULL;
	__ker_exit();
}


//
// CPU_SETUP_SIGSTUB
//
// Platforms that require special handling of parameters for __signalstub
// should define a routine and #define CPU_SETUP_SIGSTUB to the name of the
// routine.  Platforms that can use the common routine (below) can do nothing
// and the common routine will be used.
//
// void CPU_SETUP_SIGSTUB(THREAD* thp, uintptr_t new_sp, uintptr_t siginfo);
//
#ifndef CPU_SETUP_SIGSTUB

// This macro captures the default mechanism for setting up the thread
// environment for transfer to the signal handler through __signalstub.

#define CPU_SETUP_SIGSTUB(thp, new_sp, siginfo) \
	SETKIP(thp, thp->process->sigstub); \
	SETKSP(thp, new_sp); \
	SETKSTATUS(thp, siginfo);

#endif


static const struct fault_handlers sigstack_fault_handlers = {
	sigstack_fault, 0
};

typedef struct {
	SIGSTACK	ss;
	ucontext_t	con;
} sigdeliver;

//
// This delivers the signal to the thread.
// It is called from specret so it us preemptable and can
// access user address spaces.
//
void rdecl signal_specret(THREAD *thp) {
	int			 flags;
	int			 n, signo;
	SIGHANDLER	*shp;
	sigdeliver	*ssp;
	PULSE		*pup;
	PULSE		*sigpup;
	PULSE		*next;
	PROCESS		*prp = thp->process;
	uintptr_t	 old_sp;
	uintptr_t	 new_sp;
	unsigned	 sig_code;

	//if there is no thread, there is no signal.
	if(thp == NULL){
		return;
	}
	signo = 0;
	sigpup = NULL;
	thp->internal_flags &= ~_NTO_ITF_SSTEP_SUSPEND;
	flags = thp->flags & ~_NTO_TF_SIG_ACTIVE;

	pup = pril_first(&thp->sig_pending);
	for( ;; ) {
		if(pup == NULL) break;
		n = PRI_TO_SIG(pup->priority);
		if(SIG_TST(thp->sig_blocked, n) == 0) {
			if(sigpup != NULL) {
				// A second signal in the queue that's also
				// not blocked - keep the active flag on.
				flags |= _NTO_TF_SIG_ACTIVE;
				break;
			}

			// Found a signal. Save away information we will need below.
			signo    = n;
			sigpup   = pup;

			// If debugger stops the process, don't deliver the signal
			if(prp->debugger) {
				lock_kernel();
				if((*debug_thread_signal)(thp, signo, pup->code, pup->value, pup->id)) {
					unspecret_kernel();
					return;
				}
				unlock_kernel();
				SPECRET_PREEMPT(thp);
			}
			next = pril_next(pup);
			if((next != NULL) && PRI_TO_SIG(next->priority) == signo) {
				// Another signal of the same type follows in the queue - keep
				// the active flag on.
				flags |= _NTO_TF_SIG_ACTIVE;
				break;
			}
		}
		pup = pril_next_prio(pup);
	}

	// Check for a false wakeup.
	if(sigpup == NULL) {
		// turn off the _NTO_TF_SIGACTIVE bit
		thp->flags = flags;
		return;
	}

	// Get the handler for the signal. If none, the threads history.
	shp = get_signal_handler(prp, signo, 0);
	if((shp == NULL) || (shp->handler == NULL)) {
		goto killit;
	}

	old_sp = KSP(thp);


	// Adjust esp to make room for
	// signo,code,value,handler,mask64,mutex,eax,eip,cs,eflags,[previous stuff...]
	STACK_ALLOC(ssp, new_sp, old_sp, sizeof(sigdeliver));

	// handle both case, else it leaves a small window
	if(!WITHIN_BOUNDRY(old_sp, new_sp, thp->process->boundry_addr) ||
		!WITHIN_BOUNDRY(new_sp, old_sp, thp->process->boundry_addr)) {
		goto killit;
	}

	SET_XFER_HANDLER(&sigstack_fault_handlers);

	// Set signal information.
	memset(&ssp->ss.info, 0, sizeof(ssp->ss.info));

	// Have to make sure that all the fields of the context structure
	// are writable (stack pointer could be a strange value that's
	// partially writable and partially not). We do that by writing
	// into the last byte of the sigdeliver structure (we've already
	// written into the first byte above). This code is assuming that the
	// sigdeliver structure is less than a page in size and can be at most
	// in two different pages.
	((uint8_t *)&ssp[1])[-1] = 0;

	ssp->ss.info.handler = shp->handler;
	ssp->ss.info.siginfo.si_signo = signo;
//	ssp->ss.info.siginfo.si_errno = 0;
	ssp->ss.info.siginfo.si_code = sigpup->code;
	if(sigpup->code <= SI_USER) {
		ssp->ss.info.siginfo.si_pid = sigpup->id;
		ssp->ss.info.siginfo.si_uid = -1;		// No user support yet
		ssp->ss.info.siginfo.si_value.sival_int = sigpup->value;
	} else {
		ssp->ss.info.siginfo.si_fltno = sigpup->id & ~FLTNO_BDSLOT_INDICATOR;
		if(sigpup->id & FLTNO_BDSLOT_INDICATOR) {
			ssp->ss.info.siginfo.si_bdslot = 1;
		}
		ssp->ss.info.siginfo.si_fltip = (void *)KIP(thp);
		ssp->ss.info.siginfo.si_addr = (void *)sigpup->value;
	}
	_TRACE_COMM_EMIT_SIGNAL(thp, &ssp->ss.info.siginfo);
	ssp->ss.info.context = &ssp->con;

	//NYI: have to fill in other fields in ucontext_t structure
	memset(&ssp->con, 0, offsetof(ucontext_t, uc_mcontext));

	SET_XFER_HANDLER(NULL);

	SETREGIP(&ssp->con.uc_mcontext.cpu, KIP(thp));
	SETREGSP(&ssp->con.uc_mcontext.cpu, old_sp);

	// Save current execution state and blocked mask for possible restoration
	// when the signal handler returns (via ker_signal_return).
	if(thp->flags & _NTO_TF_SIGSUSPEND) {
		flags &= ~_NTO_TF_SIGSUSPEND;
		ssp->ss.sig_blocked = thp->args.ss.sig_blocked;
	} else {
		ssp->ss.sig_blocked = thp->sig_blocked;
	}

	// If we were in STATE_MUTEX or STATE_CONDVAR we need to reaquire the mutex
	// upon return from the signal handler.
	if(thp->flags & _NTO_TF_ACQUIRE_MUTEX) {
		flags &= ~_NTO_TF_ACQUIRE_MUTEX;
		ssp->ss.mutex = thp->args.mu.mutex;
		ssp->ss.mutex_timeout_flags = thp->args.mu.saved_timeout_flags;
		ssp->ss.mutex_acquire_incr = thp->args.mu.incr;
	} else {
		ssp->ss.mutex = NULL;
		ssp->ss.mutex_timeout_flags = 0;
	}

	// If a timeout is in effect push its state and remove it.
	if((ssp->ss.timeout_flags = thp->timeout_flags)  &&  thp->timeout) {
		ssp->ss.timeout_time = thp->timeout->itime.nsec;
		ssp->ss.timeout_event = thp->timeout->event;
	}

	// Remember the "error set" flag.
	ssp->ss.old_flags = flags;
	flags &= ~_NTO_TF_KERERR_SET;

	// Save away some key registers so we can restore them on handler return.
	cpu_signal_save(&ssp->ss, thp);

	// We now start changing state info and are not restartable.
	lock_kernel();

	_TRACE_NOARGS(thp);
	// Change return address to point at the handler and set up the stack and
	// registers for the transfer through __signalstub.
	CPU_SETUP_SIGSTUB(thp, new_sp, (uintptr_t)&ssp->ss.info);

	thp->flags = flags;
	thp->timeout_flags = 0;

	pulse_remove(thp->process, &thp->sig_pending, sigpup);

	// If requested set handler to SIG_DFL and clear SA_SIGINFO
	// Ignore if SIGILL or SIGTRAP as per UNIX98
	if((shp->flags & SA_RESETHAND) && signo != SIGILL && signo != SIGTRAP) {
		shp->handler = NULL;
		SIG_CLR(prp->sig_queue, signo);
	}

	// We block the signal before entering the handler and apply sigaction mask.
	if(!(shp->flags & (SA_NODEFER | SA_RESETHAND))) {
		SIG_SET(thp->sig_blocked, signo);
	}
	SIGMASK_SET(&thp->sig_blocked, &shp->sig_blocked);
	return;

killit:
	lock_kernel();
	thp->flags = flags;

	if(sigpup->code > SI_USER) {
		// if the signal is a fault is from the kernel, see if the
		// kernel debugger is interested

		sig_code = (((sigpup->code & 0xff) << 8) | signo);
		// See if we want to get the kernel debugger involved
		if(prp->flags & _NTO_PF_RING0) {
			// set SIGCODE_TRAP so kernel debugger will handle:
			// - procnto faults
			// - termer faults where no signal handler is installed
			// - loader SIGTRAP faults (ie. single stepping)
			//
			// Other loader faults are treated as SIGCODE_USER so that
			// the kernel debugger will ignore them by default.
			//
			if(!(prp->flags & _NTO_PF_LOADING) || signo == SIGTRAP) {
				sig_code |= SIGCODE_PROC;
			} else {
				sig_code |= SIGCODE_USER;
			}
		} else {
			sig_code |= SIGCODE_USER;
		}
		if((kdebug_enter(prp, sig_code, &thp->reg)) == 0) {
			pulse_remove(prp, &thp->sig_pending, sigpup);
			return;
		}
	}

	prp->valid_thp = thp;			// Just for information later
	prp->siginfo.si_signo = signo;
	prp->siginfo.si_code = sigpup->code;
	if(sigpup->code <= SI_USER) {
		prp->siginfo.si_pid = sigpup->id;
		prp->siginfo.si_uid = -1;		// No user support yet
	} else {
		// There used to be a "if(prp->siginfo.si_fltno != sigpup->id)"
		// on the above "else". This led to PR18200 where we didn't
		// fill in the prp->siginfo, though neither peterv or I (bstecher)
		// could figure out why prp->siginfo.si_fltno != 0 in the case
		// we had. We're of the opinion that the "else if..." is no longer
		// need due to changes in the process termination code.

		prp->siginfo.si_fltno = sigpup->id & ~FLTNO_BDSLOT_INDICATOR;
		if(sigpup->id & FLTNO_BDSLOT_INDICATOR) {
			prp->siginfo.si_bdslot = 1;
		}
		prp->siginfo.si_fltip = (void *)KIP(thp);
		prp->siginfo.si_addr = (void *)sigpup->value;
	}
	_TRACE_COMM_EMIT_SIGNAL(thp, &prp->siginfo);

	if(((thp->flags & _NTO_TF_NOMULTISIG)  &&  prp->num_active_threads > 1)
			|| prp->pid == SYSMGR_PID) {
		thp->status = 0;
		thread_destroy(thp);
		return;
	}


	thp->flags &= ~_NTO_TF_SPECRET_MASK;

	// process manager may want to coredump, don't do one if we're in
	// the termer thread.
	if((procmgr.process_coredump != NULL) && !(prp->flags & _NTO_PF_TERMING)) {
		struct sigevent				ev;

		if(procmgr.process_coredump(prp, &ev)) {
			if(prp->flags & _NTO_PF_COREDUMP) {
				// We have two dump requests for the same process.
				crash();
			}
			prp->flags |= _NTO_PF_COREDUMP;
			//
			// Turning on _NTO_IF_RCVPULSE will put anybody who attempts
			// to MsgSend to this process into the uncommon code path
			// - that of no threads waiting to receive. There we have a
			// test to see if we're coredumping and fail the send if
			// so. Prevents a deadlock if/when the dumper process attempts
			// to MsgSend to the process that needs to be dumped without
			// slowing down normal message passing.
			//
			stop_threads(prp, 0, _NTO_ITF_RCVPULSE);
			// sigevent_proc() may change active
			sigevent_proc(&ev);
			return;
		}
	}

	// We force the thread to kill all its sibling threads.
	thread_destroyall(thp);
}



//
// Signals are ignored at the process level. When ignored we must remove
// pending signals at the process level (they are blocked by all threads)
// and at each thread in the process. This means removing it from the queue.
// If after removing any pending signal no signals are unblocked (active)
// then we clear the active signal flags bits in the thread flags.
//
void rdecl
signal_ignore(PROCESS *prp, int signo) {
	// Set signal ignore flag and remove process pending signals.
	SIG_SET(prp->sig_ignore, signo);
	signal_remove(prp, signo);
}


//
// Discard one queued signal that matched the given siginfo.
//
void rdecl
signal_clear_thread(THREAD *thp, siginfo_t *sip) {
	PULSE		*pup;
	PULSE		*next_prio;
	PULSE		*next;
	int 		priority = SIG_TO_PRI(sip->si_signo);
	unsigned	state = 0;

#define CT_CLEARED		0x01
#define CT_PRIO_DONE	0x02
#define CT_PRIO_ACT		0x04
#define CT_SIG_ACT		0x08

	for(pup = pril_first(&thp->sig_pending); pup != NULL; pup = next_prio) {
		if(SIG_TST(thp->sig_blocked, PRI_TO_SIG(pup->priority))) {
			state &= ~CT_PRIO_ACT;
		} else {
			state |= CT_PRIO_ACT;
		}
		next_prio = pril_next_prio(pup);
		// If it's the signal number that we're looking for and we haven't
		// already cleared an entry, look for something matching *sip.
		if((priority == pup->priority) && !(state & CT_CLEARED)) {
			unsigned	count = pup->count;

			state &= ~CT_PRIO_DONE;
			for(;;) {
				next = pril_next(pup);
				if((next != NULL) && (next->priority == priority)) {
					count += next->count;
				} else {
					// We're on the last entry to check for removal
					state |= CT_PRIO_DONE;
				}
				if((sip->si_code == pup->code)
					&& (sip->si_value.sival_int == pup->value)
					&& (sip->si_pid == pup->id)) {
					// Remove signal object.
					pulse_remove(thp->process, &thp->sig_pending, pup);
					--count;
					state |= CT_CLEARED;
					break;
				}
				if(state & CT_PRIO_DONE) break;
				pup = next;
			}
			if(count == 0) {
				// There aren't any entries left for this signal number,
				// so it obviously can't have anything active.
				state &= ~CT_PRIO_ACT;
			}
		}
		if(state & CT_PRIO_ACT) {
			// If the signal number had active entries, record the global
			// state that there are active signal in the pending queue
			state |= CT_SIG_ACT;
		}
		// If we've cleared an entry and there are already active signals
		// indicated (CT_SIG_ACT and CT_CLEARED are both on) we could
		// abort the loop and just return, but there's a maximum of
		// 128 times that we're going around at that point, so I don't
		// think it's worth the extra code....
	}
	if(!(state & CT_SIG_ACT)) {
		// Went all through the list without finding anything active...
		thp->flags &= ~_NTO_TF_SIG_ACTIVE;
	}
}


//
// When we change the blocked mask on a thread we might make some signals
// active (unblocked). In this case we may need to wakeup a blocked thread
// so it can run. The decision to kill the thread or run a signal handler
// is made at the point the thread is scheduled to run (active thread).
// If a signal is unblocked and we have pending bits for the signal at the
// process level we move them to the thread level. Once moved they stay with
// the thread and are never moved back to the process.
// The thread signal active flags must be re-evaluated based upon the new
// blocked mask.
//
void rdecl
signal_block(THREAD *thp, sigset_t *sig_blocked) {
	PULSE	*pup;
	PULSE	*next;
	PROCESS *prp = thp->process;

	// Assign the new blocked mask and the saved mask if asked for.
	SIGMASK_CPY(&thp->sig_blocked, sig_blocked);

	// Don't allow these signals to be blocked.
	SIGMASK_NO_KILLSTOP(&thp->sig_blocked);

	// Check to see if the new mask allows signals to be moved from the
	// process to this thread.

	// Move signals
	//
	// If the amount of entries being transfered ever became a latency
	// issue, we could do a better job by adding a pril routine that
	// identified the start/end of each of the priorities (signo's)
	// and move them in one block:
	//	for each signo {
	//	  if(!(SIG_TST(thp->sig_blocked, signo)) {
	//	    pril_extract_range(&prp->sig_pending, SIG_TO_PRI(signo), &head, &tail);
	//	    pril_insert_range(&thp->sig_pending, head, tail);
	//	    pril_extract_range(&prp->sig_pending, SIG_TO_PRI(signo) | TYPE_FLAG_FIRST, &head, &tail);
	//	    pril_insert_range(&thp->sig_pending, head, tail);
	//	  }
	//	}
	// We'll wait until it's shown as an problem before writing the
	// code though...
	//
	for(pup = pril_first(&prp->sig_pending) ; pup != NULL ; pup = next) {
		next = pril_next(pup);
		if(!SIG_TST(thp->sig_blocked, PRI_TO_SIG(pup->priority))) {
			pril_rem(&prp->sig_pending, pup);
			pril_add(&thp->sig_pending, pup);
		}
	}

	// Check if any signals are now active.
	thp->flags &= ~_NTO_TF_SIG_ACTIVE;
	for(pup = pril_first(&thp->sig_pending); pup != NULL; pup = next) {
		int signo;
		int status;

		next = pril_next(pup);
		signo = PRI_TO_SIG(pup->priority);
		if(!SIG_TST(thp->sig_blocked, signo)) {
			if(signo == SIGCONT) {
				SIGHANDLER *shp;

				// If there is no handler, discard the SIGCONT.
				// signal_kill_process() had already unblocked the process but
				// SIGCONT will be queued if it was blocked and not ignored
				if((shp = get_signal_handler(prp, signo, 0)) && shp->handler) {
					status = -1;
				} else {
					status = SIGSTAT_QUEUED;
				}
			} else {
				status = special_case_signalhandling(prp, thp, signo, pup->code, pup->value, pup->id);
			}
			if (status == -1) {
				thp->flags |= _NTO_TF_SIG_ACTIVE;
            	// cannot break out yet, since there might be multiple
            	// signals pending, more than one of which need special
            	// case handling
			} else { // this one's been dealt with, so remove it
				pril_rem(&thp->sig_pending, pup);
				object_free(thp->process, &pulse_souls, pup);
			}
		}
	}

	// If the thread is already ready then calling force_ready is a no-op.
	if(thp->flags & _NTO_TF_SIG_ACTIVE) {
		force_ready(thp, EINTR);
	}
}


//
// Get a pointer to a signal handler object for a process.
// If it does not exist and create is set then make one.
//
SIGHANDLER *
get_signal_handler(PROCESS *prp, int signo, int create) {
	SIGTABLE *stp;
	unsigned index;	// This must be unsigned to allow a single check below.

	--signo;	// Make it 0 .. n
	// Search for a signal handler object which contains our signo.
	for(stp = prp->sig_table ; stp ; stp = stp->next) {
		index = signo - stp->sig_base;
		if(index < HANDLERS_PER_SIGTABLE) {
			return(&stp->handlers[index]);
		}
	}

	if(create) {
		// Create a new signal handler object. Each object can hold
		// more than one signal handler.
		if((stp = object_alloc(NULL, &sigtable_souls))) {
			stp->next = prp->sig_table;
			stp->sig_base = signo - signo % HANDLERS_PER_SIGTABLE;
			prp->sig_table = stp;
			return(&stp->handlers[signo - stp->sig_base]);
		}
	}

	return(NULL);
}

//
// This will acually deliver the fault to a thread
//
void rdecl
deliver_fault(THREAD *thp, siginfo_t *info) {
	SIGHANDLER	*shp;
	PROCESS 	*prp;
	PULSE		*pup;
	PULSE		*first;

	prp = thp->process;

	// If the signal is blocked then we must kill the process. Otherwise
	// we will just return to the fault and fault again and again ...
	if(SIG_TST(thp->sig_blocked, info->si_signo)) {
		// Unblock the signal and remove the handler
		SIG_CLR(thp->sig_blocked, info->si_signo);
		shp = get_signal_handler(prp, info->si_signo, 0);
		if(shp) shp->handler = NULL;
	}

	/* The PULSE entry added here doesn't have a bdslot field, so we hijack
	 * a bit in the fault value to communicate through the BD status to
	 * signal specret
	 */
	if( info->si_bdslot) {
		info->si_fltno |= FLTNO_BDSLOT_INDICATOR;
	}

	// We now add a queued signal with the invalid address as the value.
	prp->flags |= _NTO_PF_NO_LIMITS;
	pup = pulse_add(prp, &thp->sig_pending, _PULSE_PRIO_SIGNAL | _PULSE_PRIO_BOOST | SIG_TO_PRI(info->si_signo),
		info->si_code, (uintptr_t)info->si_addr, info->si_fltno);
	prp->flags &= ~_NTO_PF_NO_LIMITS;
	first = pril_first(&thp->sig_pending);
	if(pup != NULL) {
		if((pup->count > 1) || ((pup != first)
		 &&(pup->priority == first->priority)
		 &&(pup->code == first->code)
		 &&(pup->value == first->value)
		 &&(pup->id == first->id))) {
			// We have double exception - should never happen
			crash();
		 }
	}

	thp->flags |= _NTO_TF_SIG_ACTIVE;
}

unsigned kdebug_enter(PROCESS *prp, unsigned sigcode, CPU_REGISTERS *reg) {
	struct kdebug_callback			*funcs;
	unsigned						(*func)(struct kdebug_entry *entry, unsigned sigcode, void *reg);

	if((funcs = privateptr->kdebug_call) && (func = funcs->fault_entry)) {
		if(!prp && alives[KERNCPU]) {
			prp = sysmgr_prp;
		}
		sigcode = func(prp ? &prp->kdebug : 0, sigcode, reg);
		return sigcode;
	}
	if(sigcode & (SIGCODE_FATAL | SIGCODE_PROC)) {
		/* when we *have* to have some sort of kernel debugger */
		shutdown(sigcode, reg);
		return(0);
	}
	/* when we can ignore the fault */
	return sigcode;
}




//
// A thread faulted while in user space. Since the fault happened in user
// space we can pretend the fault is simple a kernel call to set a signal
// on ourselves. We cannot allow this type of fault to be ignored or blocked.
// code_signo = signo | (si_code << 8) | (fault_num << 16)
//
void rdecl
usr_fault(int code_signo, THREAD *thp, uintptr_t addr) {
	PROCESS *prp;
	siginfo_t info;

	if(thp->flags & _NTO_TF_V86) {
		//
		// A fault in V86 mode just causes the kernel call that put the
		// thread into V86 to return with an EFAULT indicator.
		//
		lock_kernel();
		thp->flags &= ~_NTO_TF_V86;
		kererr(thp, EFAULT);
		return;
	}

	prp = thp->process;

	info.si_signo = SIGCODE_SIGNO(code_signo);
	info.si_code = SIGCODE_CODE(code_signo);
	info.si_fltno = SIGCODE_FAULT(code_signo);
	info.si_bdslot = (code_signo & SIGCODE_BDSLOT) ? 1 : 0;
	info.si_addr = (void *)addr;
	info.si_fltip = (void *)KIP(thp);

	// Faults should always be faults
	if(info.si_code <= SI_USER) {
		crash();
	}

	// Check if being debugged
	if(prp->debugger) {
		lock_kernel();
		// If debugger stops the process, don't deliver the signal
		if(debug_thread_fault(thp, &info)) {
			return;
		}
	}

	lock_kernel();
	deliver_fault(thp, &info);
}

int rdecl
net_deliver_signal(THREAD *act, struct kerargs_signal_kill *kap) {
	CONNECT *cop;

	if(kap->pid == 0) {
		return ESRCH;
	}

	if(net.prp == 0  &&  net.chp == 0) {
		return ENOREMOTE;
	}

	lock_kernel();
	// Allocate a virtual conection to handle signal delivery.
	if((cop = object_alloc(act->process, &connect_souls)) == NULL) {
		return EAGAIN;
	}

	// Add connect to the servers channel vector.
	if((cop->scoid = vector_add(&net.prp->chancons, cop, 1)) == (uint16_t)-1) {
		object_free(act->process, &connect_souls, cop);
		return EAGAIN;
	}

	cop->type = TYPE_CONNECTION;
	cop->process = act->process;
	cop->links = 0;
	cop->channel = net.chp;
	cop->infoscoid = -1;
	cop->un.lcl.nd = ND_LOCAL_NODE;
	cop->un.lcl.pid = net.prp->pid;
	cop->un.lcl.chid = net.chp->chid;

	act->args.msbuff.coid = -1U;
	act->args.msbuff.dstmsglen = __KER_SIGNAL_KILL;
	act->args.msbuff.msglen = 6 * sizeof(int);
	*(int*)(&act->args.msbuff.buff[0]) = kap->nd;
	*(int*)(&act->args.msbuff.buff[4]) = kap->pid;
	*(int*)(&act->args.msbuff.buff[8]) = kap->tid;
	*(int*)(&act->args.msbuff.buff[12]) = kap->signo;
	*(int*)(&act->args.msbuff.buff[16]) = kap->code;
	*(int*)(&act->args.msbuff.buff[20]) = kap->value;

	return net_sendmsg(act, cop, act->priority);
}

__SRCVERSION("nano_signal.c $Rev: 211164 $");
