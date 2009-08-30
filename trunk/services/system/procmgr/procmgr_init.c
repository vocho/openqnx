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

char *procmgr_init_objects(void) {
	SESSION						*sep;

	if(!(sep = _scalloc(sizeof *sep))) {
		return "No Memory";
	}
	sep->links = 1;
	sep->leader = PROCMGR_PID;
	sep->pgrp = PROCMGR_PID;
	sep->fd = -2;
	procmgr_prp->session = sep;
	return 0;
}

static int procmgr_handler(message_context_t *mctp, int code, unsigned flags, void *handle) {
	int									n = ENOSYS;
	resmgr_context_t					*ctp = (resmgr_context_t *) mctp;
	union proc_msg_union_local {
		uint16_t						type;
		proc_getsetid_t					getsetid;
		proc_setpgid_t					setpgid;
		proc_wait_t						wait;
		proc_fork_t						fork;
		proc_umask_t					umask;
		proc_guardian_t					guardian;
		proc_session_t					session;
		proc_daemon_t					daemon;
		proc_event_t					event;
		sys_conf_t						conf;
		sys_cmd_t						syscmd;
	}								*msg = (union proc_msg_union_local *)(void *) ctp->msg;

	ctp->status = 0;
	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->type);
	}
	switch(msg->type) {
	case _PROC_GETSETID:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			n = EENDIAN;
		} else {
			n = procmgr_getsetid(ctp, &msg->getsetid);
		}
		break;

	case _PROC_SETPGID:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			n = EENDIAN;
		} else {
			n = procmgr_setpgid(ctp, &msg->setpgid);
		}
		break;

	case _PROC_WAIT:
		n = procmgr_wait(ctp, &msg->wait);
		break;

	case _PROC_FORK:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			n = EENDIAN;
		} else {
			n = procmgr_fork(ctp, &msg->fork);
		}
		break;

	case _PROC_SPAWN:
		n = procmgr_spawn(ctp, msg, NULL);
		break;

	case _PROC_POSIX_SPAWN:
		n = procmgr_pspawn(ctp, msg);
		break;

	case _PROC_UMASK:
		n = procmgr_umask(ctp, &msg->umask);
		break;

	case _PROC_GUARDIAN:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			n = EENDIAN;
		} else {
			n = procmgr_msg_guardian(ctp, &msg->guardian);
		}
		break;

	case _PROC_SESSION:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			n = EENDIAN;
		} else {
			n = procmgr_msg_session(ctp, &msg->session);
		}
		break;

	case _PROC_DAEMON:
		n = procmgr_msg_daemon(ctp, &msg->daemon);
		break;

	case _PROC_EVENT:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			n = EENDIAN;
		} else {
			n = procmgr_event(ctp, &msg->event);
		}
		break;

	case _PROC_RESOURCE:
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			n = EENDIAN;
		} else {
			n = procmgr_msg_resource(ctp, msg);
		}
		break;

	case _SYS_CONF:
		n = sysmgr_conf(ctp, &msg->conf);
		break;

	case _SYS_CMD:
		n = sysmgr_cmd(ctp, &msg->syscmd);
		break;

	case _SYS_VENDOR:
		if ( sys_vendor_handler_hook ) {
			n = sys_vendor_handler_hook(ctp);
		}
		break;

	default:
		break;
	}

	return proc_status(ctp, n);
}

static void proc_term(PROCESS *prp, struct sigevent *evp) {
	evp->sigev_notify = SIGEV_PULSE;
	evp->sigev_coid = PROCMGR_COID;
	evp->sigev_value.sival_int = prp->pid;
	evp->sigev_priority = prp->terming_priority;
	evp->sigev_code = PROC_CODE_TERM;
}

static int proc_start(PROCESS *prp, uintptr_t *start_ip) {
	THREAD						*thp;
	struct loader_context		*lcp = prp->lcp;

	if(!(thp = vector_lookup(&prp->threads, lcp->tid - 1)) || thp->state != STATE_REPLY) {
		return EL3HLT;
	}

	if((lcp->state & LC_STATE_MASK) == LC_FORK) {
		THREAD							*pthp;
		int								tid;
		struct _thread_local_storage	*tsp;
		char							buff[0x100];
		struct _thread_local_storage	tls;
		unsigned						size = 0;
		uintptr_t						sp;

		if(!(lookup_rcvid(0, lcp->rcvid, &pthp)) || pthp->state != STATE_REPLY) {
			return EL3HLT;
		}
		sp = GetSp(pthp);

		tid = thp->un.lcl.tls->__tid;
		tsp = thp->un.lcl.tls = pthp->un.lcl.tls;
		thp->un.lcl.stacksize = pthp->un.lcl.stacksize; 
		if(!(lcp->flags & _FORK_ASPACE)) {
			memcpy(&tls, tsp, sizeof(tls));
#ifdef STACK_GROWS_UP
			if ((size = sp - lcp->start.stackaddr) >= sizeof(buff)) return ENOMEM;
			memcpy(buff, (void *)lcp->start.stackaddr, size);
#else
			if ((size = lcp->start.stackaddr - sp) >= sizeof(buff)) return ENOMEM;
			memcpy(buff, (void *)sp, size);
#endif
		}

		KerextLock();

		tsp->__pid = prp->pid;
		tsp->__tid = tid;
		tsp->__owner = ((tsp->__pid << 16) | tsp->__tid) & ~_NTO_SYNC_WAITING;

		thp->reg = pthp->reg;
		thp->runmask = thp->default_runmask = pthp->default_runmask;

		prp->flags |= _NTO_PF_FORKED;

		if(!(lcp->flags & _FORK_ASPACE)) {
			struct vfork_info				*vip;

			if(size > sizeof buff || !(vip = prp->vfork_info = malloc(offsetof(struct vfork_info, frame) + size))) {
				return ENOMEM;
			}
			vip->rcvid = lcp->rcvid;
			vip->tls = tls;
#ifdef STACK_GROWS_UP
			vip->frame_base = (void *)lcp->start.stackaddr;
#else
			vip->frame_base = (void *)sp;
#endif
			vip->frame_size = size;
			memcpy(vip->frame, buff, size);
		}

		if(!(lcp->flags & _FORK_NOZOMBIE)) {
			prp->flags &= ~_NTO_PF_NOZOMBIE;
		}

		// Don't allow anything to change the IP
		thp->flags |= _NTO_TF_KERERR_SET;
		return EOK;
	}

	if(lcp->flags & SPAWN_EXEC) {
		struct _thread_local_storage *tsp = thp->un.lcl.tls;

		/* make sure to propagate _NTO_PF_NOZOMBIE for execs */
		if (prp->flags & _NTO_PF_NOZOMBIE)
			lcp->flags |= SPAWN_NOZOMBIE;

		tsp->__pid = lcp->ppid;
		tsp->__owner = ((tsp->__pid << 16) | tsp->__tid) & ~_NTO_SYNC_WAITING;
	}
	KerextLock();

	if(lcp->msg.spawn.i.parms.flags & SPAWN_EXPLICIT_CPU) {
		unsigned runmask;

		// Runmask has already been validated in loader_load
		runmask = lcp->msg.spawn.i.parms.runmask;
		thp->runmask = thp->default_runmask = ~runmask;
	} else {
		THREAD		*pthp;

		// Look up spawning thread and get its runmask
		if(!(lookup_rcvid(0, lcp->rcvid, &pthp)) || pthp->state != STATE_REPLY) {
			// I'm not sure how we could ever get here.
#ifndef NDEBUG
			crash();
#endif
		} else {
			thp->default_runmask = pthp->default_runmask;
			if(lcp->flags & SPAWN_EXEC) {
				/*
				 * Both masks are propagated as is across
				 * exec() so no changes are made (replacement
				 * vs parent / child relationship).
				 */
				thp->runmask = pthp->runmask;
			} else {
				thp->runmask = pthp->default_runmask;
			}
		}
	}

	*start_ip = lcp->start.eip;
	prp->initial_esp = lcp->start.esp;

	// Don't allow anything to change the IP
	thp->flags |= _NTO_TF_KERERR_SET; 

	// Propagate zombie inhibition
	if(!(lcp->flags & SPAWN_NOZOMBIE)) {
		prp->flags &= ~_NTO_PF_NOZOMBIE;
	}

	// If created under a debugger, arrange to stop before running
	if(lcp->flags & SPAWN_HOLD) {
		stop_threads(prp, 0, 0);
		prp->flags |= _NTO_PF_STOPPED;
	}

#ifndef NKDEBUG
	// Same idea, but kernel debugging
	if((lcp->flags & SPAWN_DEBUG)) {
		(void)kdebug_watch_entry(&prp->kdebug, lcp->start.eip);
	}
#endif

	return EOK;
}

static int proc_core_code;

static int proc_core(PROCESS *prp, struct sigevent *evp) {
	if(prp->flags & (_NTO_PF_NOCOREDUMP|_NTO_PF_LOADING)) {
		return 0;
	}

	switch(prp->siginfo.si_signo) {
	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGABRT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
	case SIGXCPU:
	case SIGXFSZ:
		/*
		 * These signals should generate a dump file....
		 */
		evp->sigev_notify = SIGEV_PULSE;
		evp->sigev_coid = PROCMGR_COID;
		evp->sigev_value.sival_int = prp->pid;
		evp->sigev_priority = prp->terming_priority;
		evp->sigev_code = proc_core_code;
		return 1;

	default:
		break;
	}
	return 0;
}

static int proc_stop_or_cont(PROCESS *prp, int signo, int sigcode, int sigval, int sender, struct sigevent *evp) {
	PROCESS				*parent;
	struct wait_entry	*wap;
	int					type;

	parent = prp->parent;
	if(parent == NULL) return 0;
	wap = parent->wap;
	type = (signo == SIGCONT) ? WCONTINUED: WSTOPPED;
	for( ;; ) {
		if(wap == NULL) return 0;
		if(wap->options & type) break;
		wap = wap->next;
	}

	evp->sigev_notify = SIGEV_PULSE;
	evp->sigev_coid = PROCMGR_COID;
	evp->sigev_value.sival_int = prp->pid;
	evp->sigev_priority = prp->terming_priority;
	evp->sigev_code = (signo == SIGCONT) ? PROC_CODE_CONT : PROC_CODE_STOP;
	return 1;
}

void procmgr_init(void) {
	message_attr_t	mattr;
	struct _server_info info;

	procmgr_context_init();

	pulse_attach(dpp, 0, PROC_CODE_TERM, procmgr_termer, NULL);
	pulse_attach(dpp, 0, PROC_CODE_STOP, procmgr_stop_or_cont, NULL);
	pulse_attach(dpp, 0, PROC_CODE_CONT, procmgr_stop_or_cont, NULL);

	proc_core_code = pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, procmgr_coredump, NULL);
  	(void)ConnectServerInfo(SYSMGR_PID, PROCMGR_COID, &info);
  	procmgr_scoid = info.scoid;

	procmgr.process_threads_destroyed = proc_term;
	procmgr.process_start = proc_start;
	procmgr.process_coredump = proc_core;
	procmgr.process_stop_or_cont = proc_stop_or_cont;
	procmgr.process_stack_code = pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, procmgr_stack, 0);

	// Redirect _PROCMGR_* messages to proc handler
	memset(&mattr, 0x00, sizeof(mattr));
	mattr.flags = MSG_FLAG_CROSS_ENDIAN;
	message_attach(dpp, &mattr, _SYSMGR_BASE, _SYSMGR_MAX, procmgr_handler, NULL);
	message_attach(dpp, &mattr, _PROCMGR_BASE, _PROCMGR_MAX, procmgr_handler, NULL);

	(void)procmgr_init_objects();
}

__SRCVERSION("procmgr_init.c $Rev: 199085 $");
