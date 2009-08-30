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

#include <sys/wait.h>
#include <sys/neutrino.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include "externs.h"
#include "procmgr_internal.h"

SOUL	wait_souls	INITSOUL(-1, struct wait_entry,   2,  8,  ~0);

static int wait_unblock(resmgr_context_t *ctp, io_pulse_t *msg, void *ocb) {
	PROCESS						*prp;
	struct _msg_info			info;
	pid_t						pid;

	pid = ctp->info.pid;
	if(ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE) != 0) {
		/* unblock for wait over network */

		pid = pathmgr_netmgr_pid();
		if(pid == 0) {
			/* netmgr is gone */
			return _RESMGR_NOREPLY;
		}
	}

	if((prp = proc_lock_pid(pid))) {
		// msg will be NULL if we're called from wait_close_ocb()
		if(msg && ((MsgInfo(ctp->rcvid, &info) == -1) || !(info.flags & _NTO_MI_UNBLOCK_REQ))) {
			proc_unlock(prp);
			return _RESMGR_NOREPLY;
		}
		if((prp->flags & (_NTO_PF_LOADING | _NTO_PF_TERMING)) == 0) {
			struct wait_entry			*p, **pp;

			for(pp = &prp->wap; (p = *pp); pp = &p->next) {
				if(p->rcvid == ctp->rcvid) {
					*pp = p->next;

					if(prp->wap == NULL) {
						(void)_resmgr_unbind(&ctp->info);
					}
					MsgError(ctp->rcvid, EINTR);
					proc_object_free(&wait_souls, p);
					break;
				}
			}
		}
		proc_unlock(prp);
	}
	return _RESMGR_NOREPLY;
}

static const resmgr_io_funcs_t proc_wait_funcs = {
	_RESMGR_IO_NFUNCS,
	0,
	0,
	0,
	0,
	0,
	0,
	wait_unblock,
};

/*
 * Return -1
 *     This prp can never match the criteria
 * Return 0
 *     This prp does not currently match the criteria
 * Return WNOHANG 
 *     An error with wap, reply was done (i.e. a thread unblocked)
 * Return non-zero W????? 
 *     This prp matched and a reply was done (i.e. a thread unblocked)
 */
int procmgr_wait_check(PROCESS *prp, PROCESS *parent, struct wait_entry *wap, int match) {
	struct _msg_info		info;

	/*
	 * Check if this prp matched the requested group
	 */
	switch(wap->idtype) {
	case P_ALL:
		break;

	case P_PID:
		if(prp->pid != wap->id) {
			return -1;
		}
		break;

	case P_PGID:
		if(prp->pgrp != wap->id) {
			return -1;
		}
		break;

	default:
		MsgError(wap->rcvid, EINVAL);
		return WNOHANG;
	}

	if(match) {
		/*
		 * Check for match, if parent is ignoring SIGCHLD, are we the last child?
		 */
		match &= wap->options;
		if(wap->idtype == P_ALL && (prp->sibling || parent->child != prp) &&
				sigismember(&parent->sig_ignore, SIGCHLD)) {
			match &= ~WEXITED;
		}
		/*
		 * Have we already responded with exit code?
		 */
		if(!(prp->flags & _NTO_PF_WAITINFO)) {
			match &= ~WEXITED;
		}
	} else {
		/*
		 * Check if the requested prp currently matches any options
		 */
		if((wap->options & WTRAPPED) && (prp->flags & (_NTO_PF_DEBUG_STOPPED | _NTO_PF_PTRACED)) ==
				(_NTO_PF_DEBUG_STOPPED | _NTO_PF_PTRACED)) {
			match = WTRAPPED;
		} else if((wap->options & WEXITED) && (prp->flags & _NTO_PF_WAITINFO)) {
			match = WEXITED;
		} else if((wap->options & WCONTINUED) && (prp->flags & _NTO_PF_CONTINUED)) {
			match = WCONTINUED;
		} else if((wap->options & WUNTRACED) && (prp->flags & _NTO_PF_STOPPED) &&
				prp->siginfo.si_signo != 0) {
			match = WUNTRACED;
		}
	}

	/*
	 * If no options matched, check if it could ever match options
	 */
	if(match == 0) {
		int				options = wap->options;
        
		if(prp->flags & (_NTO_PF_ZOMBIE | _NTO_PF_TERMING)) {
			options &= ~(WUNTRACED|WTRAPPED|WCONTINUED);
		}
		if((prp->flags & (_NTO_PF_ZOMBIE | _NTO_PF_WAITINFO)) == _NTO_PF_ZOMBIE) {
			options &= ~WEXITED;
		}
		if((prp->flags & _NTO_PF_NOZOMBIE) || sigismember(&parent->sig_ignore, SIGCHLD)) {
			options &= ~WEXITED;
		}
		if(prp->flags & _NTO_PF_WAITDONE) {
			options &= ~WEXITED;
		}
		if(!(prp->flags & _NTO_PF_PTRACED)) {
			options &= ~WTRAPPED;
		}
		if((options & (WEXITED|WUNTRACED|WTRAPPED|WCONTINUED)) == 0) {
			return -1;
		}
		return 0;
	}

	/*
	 * Unblock the waiting thread...
	 */
	
	if(MsgInfo(wap->rcvid, &info) != -1) {
		siginfo_t	siginfo;

		// unbind and unblock if rcvid is still around
		if((!parent->wap) || ((parent->wap == wap) && (parent->wap->next == NULL))) {
			(void)_resmgr_unbind(&info);
		}
		siginfo = prp->siginfo;
		if(siginfo.si_signo != SIGCHLD) {
			if(prp->flags & _NTO_PF_COREDUMP) {
				siginfo.si_code = CLD_DUMPED;
			} else {
				siginfo.si_code = CLD_KILLED;
			}
			siginfo.si_status = siginfo.si_signo;
			siginfo.si_pid = prp->pid;
			siginfo.si_signo = SIGCHLD;
		}
		if(info.flags & _NTO_MI_ENDIAN_DIFF) {
			ENDIAN_SWAP32(&siginfo.si_signo);
			ENDIAN_SWAP32(&siginfo.si_code);
			ENDIAN_SWAP32(&siginfo.si_errno);
			ENDIAN_SWAP32(&siginfo.si_pid);
			ENDIAN_SWAP32(&siginfo.si_status);
			ENDIAN_SWAP32(&siginfo.si_utime);
			ENDIAN_SWAP32(&siginfo.si_stime);
		}
		MsgReply(wap->rcvid, 0, &siginfo, sizeof siginfo);
	} else {
		KerextSlogf( _SLOG_SETCODE( _SLOGC_PROC, 0 ), _SLOG_INFO, "proc_wait_check: MsgInfo() failed, errno=%d", errno);
	}

	/*
	 * Clean up prp status if requested so it is not reported again
	 */
	if(wap->options & WNOWAIT) {
		return WNOWAIT;
	}
	switch(match) {
	case WEXITED:
		if(prp->flags & _NTO_PF_WAITINFO) {
			parent->kids_running_time += prp->running_time + prp->kids_running_time;
			parent->kids_system_time += prp->system_time + prp->kids_system_time;
			prp->flags &= ~_NTO_PF_WAITINFO;
			prp->flags |= _NTO_PF_WAITDONE;
		}
		if(prp->flags & _NTO_PF_ZOMBIE) {
			MsgSendPulse(PROCMGR_COID, prp->terming_priority, PROC_CODE_TERM, prp->pid);
		} else {
			match = WNOWAIT;
		}
		break;
	case WUNTRACED: // also WSTOPPED
		prp->siginfo.si_signo = 0;
		break;
	case WTRAPPED:
		break;
	case WCONTINUED:
		prp->flags &= ~_NTO_PF_CONTINUED;
		break;
	default:
		break;
	}
	return match;
}

int procmgr_wait(resmgr_context_t *ctp, proc_wait_t *msg) {
	PROCESS						*prp, *child;
	struct wait_entry			*wap, **pwap, waitl;
	int							alive;

	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->i.idtype);
		ENDIAN_SWAP32(&msg->i.options);
		ENDIAN_SWAP32(&msg->i.id);
	}
	if(msg->i.options & ~WOPTMASK) {
		return EINVAL;
	}
	waitl.rcvid = ctp->rcvid;
	waitl.idtype = msg->i.idtype;
	waitl.options = msg->i.options;
	waitl.id = msg->i.id;

	alive = 0;
	if(ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE) != 0) {
		struct _client_info info;
		struct _cred_info *src, *dst;
		pid_t	nm_pid;

		if(ConnectClientInfo(ctp->info.scoid, &info, 0) == -1) {
			return errno;
		}

		nm_pid = pathmgr_netmgr_pid();
		if(nm_pid == 0) {
			/* netmgr is gone */
			return EL2HLT;
		}
		if(!(prp = proc_lock_pid(nm_pid))) {
			return EL2HLT;
		}

		src = &info.cred;

		for(child = prp->child; child; child = child->sibling) {
			if(child->pid != waitl.id) {
				continue;
			}
			/* security check */
			dst = &child->cred->info;
			if(!(src->euid == 0  ||
			   src->ruid == dst->ruid  ||
			   src->ruid == dst->suid  ||
			   src->euid == dst->ruid  ||
			   src->euid == dst->suid)) {
				return proc_error(EPERM, prp);
			}
			switch(procmgr_wait_check(child, prp, &waitl, 0)) {
			case 0:
				alive++;
				break;
			case -1:
				break;
			default:	
				return proc_error(_RESMGR_NOREPLY, prp);
			}
			if(alive) {
				break;
			}
		}
		if(alive == 0) {
			return proc_error(ECHILD, prp);
		}
	} else {
		if(!(prp = proc_lock_pid(ctp->info.pid))) {
			return EL2HLT;
		}

		for(child = prp->child; child; child = child->sibling) {
			switch(procmgr_wait_check(child, prp, &waitl, 0)) {
			case 0:
				alive++;
				break;
			case -1:
				break;
			default:	
				return proc_error(_RESMGR_NOREPLY, prp);
			}
		}

		//
		// If we're the guardian process for our parent, we'll pick up his
		// children when he dies so we need to see if there are any children
		// of our parent which might satisfy the wait condition in the
		// future and, if so, pretend like they're our children for the
		// purposes of the wait request.
		//
		if(prp->parent && prp->parent->guardian == prp) {
			if(procmgr_wait_check(prp->parent, prp, &waitl, 0) == 0) {
				alive++;
			}
		}

		if(alive == 0) {
			if(!prp->parent || prp->parent->guardian != prp || procmgr_wait_check(prp->parent, prp, &waitl, 0) != 0) {
				return proc_error(ECHILD, prp);
			}
		}
	}

	if(waitl.options & WNOHANG) {
		memset(&msg->o, 0x00, sizeof msg->o);
		return proc_error(_RESMGR_PTR(ctp, &msg->o, sizeof msg->o), prp);
	}
			
	// nothing waiting, so add to queue sorted so pid match has higher priority
	if(!(wap = proc_object_alloc(&wait_souls))) {
		return proc_error(ENOMEM, prp);
	}
	for(pwap = &prp->wap; (waitl.next = *pwap); pwap = &waitl.next->next) {
		if(waitl.next->idtype < waitl.idtype) {
			break;
		}
	}
	*wap = waitl;
	*pwap = wap;

	ctp->id = root_id;
	(void)resmgr_open_bind(ctp, wap, &proc_wait_funcs);
	return proc_error(_RESMGR_NOREPLY, prp);
}

int procmgr_stop_or_cont(message_context_t *ctp, int code, unsigned flags, void *handle) {
	union sigval		value = ctp->msg->pulse.value;
	PROCESS				*prp;
	PROCESS				*parent;
	int					type;
	int					status;
	struct wait_entry	*wap, **pwap;

	if (procmgr_scoid != ctp->msg->pulse.scoid) {
		return 0;
	}

	if((prp = proc_lock_pid(value.sival_int))) {
		if((parent = proc_lock_pid(prp->parent->pid))) {
			type = (code == PROC_CODE_CONT) ? WCONTINUED : WSTOPPED;
			for(pwap = &parent->wap; (wap = *pwap);) {
				if((status = procmgr_wait_check(prp, parent, wap, type)) > 0) {
					*pwap = wap->next;
					proc_object_free(&wait_souls, wap);
					if(status == type) {
						break;
					}
				} else {
					pwap = &wap->next;
				}
			}
			proc_unlock(parent);
		}
		proc_unlock(prp);
	}
	return 0;
}

void procmgr_nozombie(PROCESS *prp, int status) {
	/*
	 * This should use CLD_EXITED instead of SI_USER, but the SignalKill()
	 * kernel call does not allow it. SI_USER is alowable though, and this
	 * is only used by QNX functions right now.
	 */
	if(!(prp->flags & _NTO_PF_NOZOMBIE)) {
		PROCESS								*parent;
		struct wait_entry					*wap, **pwap;

		prp->siginfo.si_signo = SIGCHLD;
		prp->siginfo.si_code = CLD_EXITED;
		prp->siginfo.si_pid = prp->pid;
		prp->siginfo.si_status = status;

		parent = proc_lock_parent(prp);

		if(!sigismember(&(prp->parent->sig_ignore), SIGCHLD)) {
			prp->flags |= _NTO_PF_WAITINFO;
		}
		for(pwap = &parent->wap; (wap = *pwap);) {
			if((status = procmgr_wait_check(prp, parent, wap, WEXITED)) > 0) {
				*pwap = wap->next;
				proc_object_free(&wait_souls, wap);
				if(status == WEXITED) {
					break;
				}
			} else {
				pwap = &wap->next;
			}
		}
		prp->flags |= _NTO_PF_NOZOMBIE;
		proc_unlock(parent);
		SignalKill(ND_LOCAL_NODE, prp->parent->pid, 0, SIGCHLD, SI_USER, prp->pid);
	}
}

__SRCVERSION("procmgr_wait.c $Rev: 166042 $");
