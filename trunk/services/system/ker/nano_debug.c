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

static void rdecl debug_sigset(PRIL_HEAD *ph, sigset_t *ssp) {
	PULSE	*pup;

	SIGMASK_ZERO(ssp);
	for(pup = pril_first(ph); pup != NULL; pup = pril_next(pup)) {
		if(TYPE_MASK(pup->type) == TYPE_SIGNAL) { // only TYPE_SIGNAL
			SIG_SET(*ssp, PRI_TO_SIG(pup->priority));
		}
	}
}

int rdecl debug_process(PROCESS *prp, debug_process_t *dpp) {
	CREDENTIAL					*crp;
	uint64_t					runtime;
	uint64_t					systime;

	do {
		runtime = prp->running_time;
	} while(runtime != prp->running_time);
	do {
		systime = prp->system_time;
	} while(systime != prp->system_time);
	dpp->utime = runtime - systime;
	dpp->stime = systime;
	dpp->cutime = prp->kids_running_time - prp->kids_system_time;
	dpp->cstime = prp->kids_system_time;
	dpp->start_time = prp->start_time;
	dpp->pid = prp->pid;
	dpp->parent = prp->parent ? prp->parent->pid : 0;
	dpp->flags = prp->flags;
	dpp->umask = prp->umask;
	dpp->child = prp->child ? prp->child->pid : 0;
	dpp->sibling = prp->sibling ? prp->sibling->pid : 0;
	dpp->pgrp = prp->pgrp;
	dpp->sid = 0;
	if(prp->session) {
		dpp->sid = prp->session->leader;
		if(prp->session->pgrp != prp->pgrp) {
			dpp->flags |= _NTO_PF_BKGND_PGRP;
		}
	}
	dpp->base_address = prp->base_addr;
	dpp->initial_stack = prp->initial_esp;
	if((crp = prp->cred)) {
		dpp->uid = crp->info.ruid;
		dpp->gid = crp->info.rgid;
		dpp->euid = crp->info.euid;
		dpp->egid = crp->info.egid;
		dpp->suid = crp->info.suid;
		dpp->sgid = crp->info.sgid;
	} else {
		dpp->uid =
		dpp->gid = 
		dpp->euid =
		dpp->egid =
		dpp->suid =
		dpp->sgid = 0;
	}
	dpp->sig_ignore = prp->sig_ignore;
	dpp->sig_queue = prp->sig_queue;
	debug_sigset(&prp->sig_pending, &dpp->sig_pending);
	dpp->num_chancons = prp->chancons.nentries - prp->chancons.nfree;
	dpp->num_fdcons = prp->fdcons.nentries - prp->fdcons.nfree;
	dpp->num_threads = prp->threads.nentries - prp->threads.nfree;
	dpp->num_timers = prp->timers.nentries - prp->timers.nfree;
	dpp->priority = prp->process_priority;
	dpp->pls = (uintptr_t)prp->pls;
	debug_moduleinfo(prp, NULL, &dpp->extsched);
	memset(dpp->reserved2, 0x00, sizeof dpp->reserved2);	
	dpp->sigstub = (uintptr_t)prp->sigstub;
	dpp->canstub = (uintptr_t)prp->canstub;
	memset(dpp->reserved, 0x00, sizeof dpp->reserved);	
	return EOK;
}




int rdecl debug_thread(PROCESS *prp, THREAD *thp, debug_thread_t *dtp) {
	DEBUG					*dep;
	uint64_t				runtime;

	dtp->flags = 0;
	if(thp) {
		if((thp->flags & _NTO_TF_TO_BE_STOPPED) || thp->state == STATE_DEAD ||
				thp->state == STATE_STOPPED) {
			dtp->flags |= _DEBUG_FLAG_STOPPED;
		}
	} else {
		memset(dtp, 0x00, sizeof *dtp);
		dtp->flags |= _DEBUG_FLAG_IPINVAL;
	}

	dtp->info = prp->siginfo;
	if((dep = prp->debugger)) {
		dtp->why = dep->why;
		dtp->what = dep->what;
		dtp->flags |= dep->flags;
		if(thp && dep->tid == thp->tid) {
			dtp->flags |= _DEBUG_FLAG_CURTID;
		}
	} else if(prp->flags & _NTO_PF_TERMING) {
		dtp->why = _DEBUG_WHY_TERMINATED;
		dtp->what = prp->siginfo.si_status;
	} else if((prp->flags & _NTO_PF_COREDUMP) && prp->valid_thp == thp) {
		dtp->why = _DEBUG_WHY_SIGNALLED;
		dtp->what = prp->siginfo.si_signo;
	} else {
		dtp->why = _DEBUG_WHY_REQUESTED;
		dtp->what = 0;
	}
	if(prp->pid == SYSMGR_PID) {
		dtp->flags |= _DEBUG_FLAG_ISSYS;
	}

	dtp->pid = prp->pid;
	if(!thp) {
		return EOK;
	}

	do {
		runtime = thp->running_time;
	} while(runtime != thp->running_time);
	dtp->sutime = runtime;
	dtp->start_time = thp->start_time;
	dtp->tid = thp->tid + 1;
	dtp->ip = (dtp->flags & _DEBUG_FLAG_IPINVAL) ? 0 : KIP(thp);
	dtp->sp = KSP(thp);
	dtp->stkbase = (uintptr_t)thp->un.lcl.stackaddr;
	dtp->stksize = thp->un.lcl.stacksize;
	dtp->tls = (uintptr_t)thp->un.lcl.tls;
	dtp->tid_flags = thp->flags & _NTO_TF_PUBLIC_MASK;
	dtp->priority = thp->priority;
	dtp->real_priority = thp->real_priority;
	dtp->policy = thp->policy;
	dtp->syscall = _TRACE_GETSYSCALL(thp->syscall);
	switch(dtp->state = thp->state) {
	case STATE_SEND:
	case STATE_REPLY: {
		CONNECT				*cop = (CONNECT *)thp->blocked_on;

		if(cop->flags & COF_NETCON) {
			CHANNEL				*chp;

			dtp->blocked.connect.coid = cop->un.net.coid;
			dtp->blocked.connect.nd = ND_LOCAL_NODE;
			if((chp = cop->channel)) {
				dtp->blocked.connect.pid = chp->process->pid;
				dtp->blocked.connect.chid = chp->chid;
			} else {
				dtp->blocked.connect.pid = -1;
				dtp->blocked.connect.chid = -1;
			}
		} else {
			dtp->blocked.connect.coid = thp->args.ms.coid;
			dtp->blocked.connect.nd = cop->un.lcl.nd;
			dtp->blocked.connect.pid = cop->un.lcl.pid;
			dtp->blocked.connect.chid = cop->un.lcl.chid;
		}
		dtp->blocked.connect.scoid = cop->scoid | _NTO_SIDE_CHANNEL;
		break;
	}
	case STATE_WAITTHREAD:
	case STATE_JOIN:
		dtp->blocked.join.tid = ((THREAD *)thp->blocked_on)->tid + 1;
		break;
	case STATE_SEM:
		dtp->blocked.sync.sync = 0;
		dtp->blocked.sync.id = thp->blocked_on ? ((SYNC *)thp->blocked_on)->addr : 0;
		break;	
	case STATE_RECEIVE:
		dtp->blocked.channel.chid = ((CHANNEL *)thp->blocked_on)->chid;
		break;
	case STATE_WAITPAGE:
        waitpage_status_get(thp, dtp);
		break;
	case STATE_MUTEX:
	case STATE_CONDVAR:
		dtp->blocked.sync.sync = (intptr_t)thp->args.mu.mutex;
		dtp->blocked.sync.id = thp->blocked_on ? ((SYNC *)thp->blocked_on)->addr : 0;
		break;
	case STATE_STACK:
		dtp->blocked.stack.size = (thp->flags & _NTO_TF_WAAA) ? thp->un.lcl.stacksize : 0;
		break;
	default:
		break;
	}
	dtp->last_cpu = thp->runcpu;
	dtp->timeout = thp->timeout_flags;
	dtp->last_chid = thp->last_chid;
	dtp->sig_blocked = thp->sig_blocked;

	dtp->nsec_since_block = get_nsec_since_block(thp);  
	
	debug_sigset(&thp->sig_pending, &dtp->sig_pending);
	debug_moduleinfo(prp, thp, &dtp->extsched);
	memset(dtp->reserved2, 0x00, sizeof dtp->reserved2);	
	return EOK;
}


uint64_t get_nsec_since_block(THREAD *thp) { 
	if (thp->state==STATE_READY || thp->state==STATE_RUNNING) { 
		/*not valid if thp isnt blocked */
		return (uint64_t)0; 
	} else { 
		uint64_t	snapshot;
		snap_time(&snapshot,0);
		return snapshot - thp->timestamp_last_block; 
	}
}


static int					debug_code;

static void debug_event(pid_t pid, int priority) {
	struct sigevent				event; 

	// Pulse the process manager
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = SYSMGR_COID;
	event.sigev_value.sival_int = pid;
	event.sigev_priority = priority == -1 ? actives[KERNCPU]->real_priority : priority;
	event.sigev_code = debug_code;
	sigevent_proc(&event);
}

int rdecl debug_stop(PROCESS *prp) {
	DEBUG				*dep;

	if(!(dep = prp->debugger)) {
		return EINVAL;
	}
	if(!(prp->flags & _NTO_PF_DEBUG_STOPPED)) {
		lock_kernel();
		prp->flags |= _NTO_PF_DEBUG_STOPPED;
		stop_threads(prp, 0, 0);
		dep->flags |= _DEBUG_FLAG_ISTOP;
		dep->why = _DEBUG_WHY_REQUESTED;
		dep->what = 0;
		(void)vector_search(&prp->threads, 0, (unsigned *)&dep->tid);
		debug_event(prp->pid, -1);
	}
	return EOK;
}

int rdecl debug_run(PROCESS *prp, debug_run_t *drp) {
	DEBUG						*dep;
	THREAD						*thp;
	int							tid;
	unsigned					drp_flags;

	if(!(dep = prp->debugger)) {
		return EBADF;
	}

	if(!(prp->flags & _NTO_PF_DEBUG_STOPPED)) {
		return EBUSY;
	}

	drp_flags = drp->flags;
	// Check for unsupported debug modes....
	if(drp_flags & ~(_DEBUG_RUN_VADDR | _DEBUG_RUN_TRACE | _DEBUG_RUN_FAULT |
			_DEBUG_RUN_CURTID | _DEBUG_RUN_STEP | _DEBUG_RUN_CLRSIG |
			_DEBUG_RUN_STEP_ALL | _DEBUG_RUN_CLRFLT)) {
		return ENOTSUP;
	}

	// Find thread pointer (If requested change the current tid)
	if((tid = dep->tid) && (drp_flags & _DEBUG_RUN_CURTID)) {
		tid = drp->tid - 1;
	}
	if(!(thp = vector_lookup(&prp->threads, tid))) {
		return ENXIO;
	}

	lock_kernel();

	dep->tid = tid;

	if((drp_flags & _DEBUG_RUN_CLRSIG) && dep->why == _DEBUG_WHY_SIGNALLED) {
		signal_clear_thread(thp, &prp->siginfo);
		dep->why = _DEBUG_WHY_REQUESTED;
	}

	if(dep->why == _DEBUG_WHY_FAULTED) {
		if(drp_flags & _DEBUG_RUN_CLRFLT) {
			dep->why = _DEBUG_WHY_REQUESTED;
		} else {
			deliver_fault(thp, &prp->siginfo);
		}
	}

	if(drp_flags & _DEBUG_RUN_VADDR) {
		SETKIP(thp, drp->ip);
	}

	if(drp_flags & _DEBUG_RUN_TRACE) {
		dep->signals = drp->trace;
	}
	if(drp_flags & _DEBUG_RUN_FAULT) {
		dep->faults = drp->fault;
	}

	dep->flags &= ~(_DEBUG_FLAG_ISTOP | _DEBUG_FLAG_TRACE_EXEC | _DEBUG_FLAG_TRACE_RD |
					_DEBUG_FLAG_TRACE_WR | _DEBUG_FLAG_TRACE_MODIFY | _DEBUG_FLAG_SSTEP);

	thp->internal_flags &= ~_NTO_ITF_SSTEP_SUSPEND;
	thp->flags &= ~_NTO_TF_SSTEP;

	dep->skip_brk = 0;
	if(!(thp->flags & _NTO_TF_THREADS_HOLD)) {
		BREAKPT					*d;

		for(d = dep->brk; d; d = d->next) {
			if((d->brk.type & _DEBUG_BREAK_EXEC) && KIP(thp) == d->brk.addr) {
				dep->skip_brk = d;
				if(!(drp_flags & _DEBUG_RUN_STEP)) {
					prp->flags &= ~_NTO_PF_DEBUG_STOPPED;
				}
				drp_flags = (drp_flags & ~_DEBUG_RUN_STEP_ALL) | _DEBUG_RUN_STEP;
				break;
			}
		}
	}

	if(drp_flags & (_DEBUG_RUN_STEP | _DEBUG_RUN_STEP_ALL)) {
		int						status;

		if((status = cpu_debug_sstep(dep, thp))) {
			if(status > 0) return status;
			// negative status, can't support RUN_STEP_ALL
			drp_flags = (drp_flags & ~_DEBUG_RUN_STEP_ALL) | _DEBUG_RUN_STEP;
		}
		thp->flags |= _NTO_TF_SSTEP;
	}
	if(drp_flags & _DEBUG_RUN_STEP) {
		if(thp->flags & _NTO_TF_THREADS_HOLD) {
			return EBUSY;
		}
		if(!(prp->flags & _NTO_PF_STOPPED)) {
			thp->flags &= ~_NTO_TF_TO_BE_STOPPED;
			if(thp->state == STATE_STOPPED) {
				ready(thp);
			}
		}
	} else {
		prp->flags &= ~_NTO_PF_DEBUG_STOPPED;
		cont_threads(prp, 0);
	}
	return EOK;
}

int rdecl debug_break(PROCESS *prp, debug_break_t *dbp) {
	DEBUG					*dep;
	BREAKPT					*d, **pd;
	int						status;

	if(!(dep = prp->debugger)) {
		return EBADF;
	}

	// Search for sorted matching breakpt
	for(pd = &dep->brk; (d = *pd); pd = &d->next) {
		if((dbp->type & _DEBUG_BREAK_MASK) == (d->brk.type & _DEBUG_BREAK_MASK)) {
			if(dbp->addr == d->brk.addr) {
				break;
			}
			if(dbp->addr > d->brk.addr) {
				d = 0;
				break;
			}
		} else if((dbp->type & _DEBUG_BREAK_MASK) > (d->brk.type & _DEBUG_BREAK_MASK)) {
			d = 0;
			break;
		}
	}

	// Remove existing if requested
	if(dbp->size < 0) {
		if(dbp->size != -1) {
			return EINVAL;
		}
		if(d) {
			lock_kernel();
			(void)cpu_debug_brkpt(dep, d);
			*pd = d->next;
			object_free(prp, &breakpt_souls, d);
			return EOK;
		}
		return EINVAL;
	}

	lock_kernel();
	if(d) {
		d->brk.size = -1;
		(void)cpu_debug_brkpt(dep, d);
		d->brk = *dbp;
	} else {
		// Allocate new if requested
		if(!(d = object_alloc(prp, &breakpt_souls))) {
			return ENOMEM;
		}
		d->brk = *dbp;
		d->next = *pd;
		*pd = d;
	}
	if((status = cpu_debug_brkpt(dep, d))) {
		*pd = d->next;
		object_free(prp, &breakpt_souls, d);
	}
	return status;
}

int rdecl debug_break_list(PROCESS *prp, debug_breaklist_t *dbpl) {
	DEBUG					*dep;
	BREAKPT					*d;
	int 					add_count, total_count;

	if(!(dep = prp->debugger)) {
		return EBADF;
	}

	add_count = 0;
	total_count = 0;

	// Search for sorted matching breakpt
	for(d = dep->brk; d; d = d->next) {
		// the intention is to add support for a breakpoint ossfet into the list
		if(add_count < dbpl->count && total_count >= dbpl->offset) {
			memcpy(&dbpl->breakpoints[add_count], &d->brk, sizeof(d->brk));
			add_count++;

			//Only go through as many items as we need to
			if(add_count == dbpl->count && dbpl->offset != 0) {
				break;
			}
		}
		total_count++;
	}

	dbpl->count = add_count;
	dbpl->total_count = total_count;

	return EOK;
}

static int rdecl process_exit(PROCESS *prp, int priority) {
	DEBUG						*dep;

	if(!(dep = prp->debugger)) {
		return 0;
	}

	prp->flags |= _NTO_PF_DEBUG_STOPPED;
	dep->flags |= _DEBUG_FLAG_ISTOP;
	dep->why = _DEBUG_WHY_TERMINATED;
	dep->what = prp->siginfo.si_status;
	memset(&prp->siginfo, 0x00, sizeof prp->siginfo);
	debug_event(prp->pid, priority);
	return 1;
}

static int rdecl handle_signal(PROCESS *prp, int tid, int signo, int sigcode, int sigval, int sender) {
	DEBUG						*dep;

	if(!(dep = prp->debugger)) {
		return 0;
	}

	// Is there any interest in the fault?
	if(!SIG_TST(dep->signals, signo)) {
		return 0;
	}

	// set a process flag stating it is debug stopped.
	prp->flags |= _NTO_PF_DEBUG_STOPPED;
	stop_threads(prp, 0, 0);

	// Save away fault state
	dep->flags |= _DEBUG_FLAG_ISTOP;
	dep->why = tid == -1 ? _DEBUG_WHY_JOBCONTROL : _DEBUG_WHY_SIGNALLED;
	dep->what = prp->siginfo.si_signo = signo;
	prp->siginfo.si_code = sigcode;
	prp->siginfo.si_value.sival_int = sigval;
	prp->siginfo.si_pid = sender;
	prp->siginfo.si_errno = 0;
	if(tid > -1) {
		dep->tid = tid;
	}
	debug_event(prp->pid, -1);
	return 1;
}

static int rdecl thread_signal(THREAD *thp, int signo, int sigcode, int sigval, int sender) {
	return handle_signal(thp->process, thp->tid, signo, sigcode, sigval, sender);
}

static int rdecl process_stopped(PROCESS *prp, int signo, int sigcode, int sigval, int sender) {
	if(signo != SIGSTOP && signo != SIGTSTP && signo != SIGTTIN && signo != SIGTTOU) {
		return 0;
	}
	return handle_signal(prp, -1, signo, sigcode, sigval, sender);
}

static int rdecl thread_fault(THREAD *thp, siginfo_t *info) {
	DEBUG						*dep;
	PROCESS						*prp;
	unsigned					flags;
	int							skip, ret;

	prp = thp->process;
	if(!(dep = prp->debugger)) {
		return 0;
	}

	/* check the status of the fault first up */
	flags = 0;
	ret = cpu_debug_fault(dep, thp, info, &flags );

	// Insert skip breakpoint, and make all threads runnable if nessessary
	skip = 0;
	if(dep->skip_brk) {
		debug_detach_brkpts(dep);
		dep->skip_brk = 0;
		debug_attach_brkpts(dep);
		if(!(prp->flags & _NTO_PF_DEBUG_STOPPED)) {
			skip = _DEBUG_FLAG_SSTEP;
			cont_threads(prp, 0);
		}
	}

	if(ret) {
		return 1;
	}

	if(flags & skip) {
		return 1;
	}

	if(info->si_code <= SI_USER) {
		crash();
	}

	thp->flags &= ~_NTO_TF_SSTEP;

	// Is there any interest in the fault?
	if(!SIG_TST(dep->faults, info->si_fltno)) {
		return 0;
	}

	_TRACE_SYS_EMIT_ADDRESS(thp, KIP(thp));

	prp->flags |= _NTO_PF_DEBUG_STOPPED;
	stop_threads(prp, 0, 0);

	// Save away fault state
	dep->flags |= _DEBUG_FLAG_ISTOP | (flags &
		(_DEBUG_FLAG_SSTEP | _DEBUG_FLAG_TRACE_EXEC |
		_DEBUG_FLAG_TRACE_RD | _DEBUG_FLAG_TRACE_WR | _DEBUG_FLAG_TRACE_MODIFY));
	dep->why = _DEBUG_WHY_FAULTED;
	prp->siginfo = *info;
	dep->what = info->si_fltno;
	dep->tid = thp->tid;

	// Don't enqueue a signal for the fault.
	debug_event(prp->pid, thp->priority);
	return 1;
}

void DebugInstall(int code) {
	debug_code = code;
	debug_process_exit = process_exit;
	debug_process_stopped = process_stopped;
	debug_thread_signal = thread_signal;
	debug_thread_fault = thread_fault;
	debug_attach_brkpts = cpu_debug_attach_brkpts;
	debug_detach_brkpts = cpu_debug_detach_brkpts;
}

__SRCVERSION("nano_debug.c $Rev: 169879 $");
