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
#include <share.h>
#include <sys/dcmd_chr.h>
#include <sys/procmgr.h>
#include <sys/procmsg.h>

/*
 * Detach the session, and return an fd to close
 * if nessessary (-1 indicates no fd needs to be closed)
 */
int procmgr_sleader_detach(PROCESS *prp) {
	int					fd = -1;

	if(prp->flags & _NTO_PF_SLEADER) {
		SESSION				*sep;
	
		prp->flags &= ~_NTO_PF_SLEADER;
		if((sep = prp->session)) {
			fd = sep->fd;
			sep->fd = -1;
			if(sep->pgrp > 0) {		// Posix.1 3.2.2.2 item 6
				SignalKill(ND_LOCAL_NODE, -sep->pgrp, 0, SIGHUP, SI_USER, 0);
			}
			sep->pgrp = -1;
		}
	}
	return fd;
}


int procmgr_msg_session(resmgr_context_t *ctp, proc_session_t *msg) {
	PROCESS						*prp;
	SESSION						*sep;

	switch(msg->i.event) {
	case PROCMGR_SESSION_TCSETSID: {
		io_openfd_t					io_openfd;
		int							fd;
		int							status;

		if(!(prp = proc_lock_pid(ctp->info.pid))) {
			return EL2HLT;
		}

		if(!(prp->flags & _NTO_PF_SLEADER) || !(sep = prp->session) || msg->i.sid != sep->leader) {
			return proc_error(EPERM, prp);
		}

		if(proc_thread_pool_reserve() != 0) {
			return proc_error(EAGAIN, prp);
		}

		if(msg->i.id == -1) {
			fd = sep->fd;
			sep->fd = -1;
			close(fd);
			proc_thread_pool_reserve_done();
			return proc_error(EOK, prp);
		}

		// We know the process won't die because we haven't replied so sep stays valid
		proc_unlock(prp);

		memset(&io_openfd.i, 0x00, sizeof io_openfd.i);

		// If we have a controlling terminal, and the managing server is still alive 
		// then error out can't have two controlling terminals.  If we detect the 
		// server has disappeared, the set sep->fd = -1;
		if(sep->fd != -1) {
			if (ConnectServerInfo(0, sep->fd, &io_openfd.i.info) == sep->fd) {
				proc_thread_pool_reserve_done();
				return EPERM;
			} else {
				fd = sep->fd;
				sep->fd = -1;
				close(fd);	
			}
		}

		// Get information on passed fd
		if(ConnectServerInfo(ctp->info.pid, msg->i.id, &io_openfd.i.info) != msg->i.id) {
			proc_thread_pool_reserve_done();
			return EBADF;
		}

		// make a connection to the channel
		if((fd = ConnectAttach(io_openfd.i.info.nd, io_openfd.i.info.pid, io_openfd.i.info.chid, 0, 0)) == -1) {
			proc_thread_pool_reserve_done();
			return EINVAL;
		}

		// send the openfd message (no share flags)
		io_openfd.i.type = _IO_OPENFD;
		io_openfd.i.combine_len = sizeof io_openfd.i;
		io_openfd.i.ioflag = _IO_FLAG_RD | _IO_FLAG_WR;
		io_openfd.i.sflag = SH_DENYNO;
		io_openfd.i.xtype = _IO_OPENFD_NONE;
		io_openfd.i.info.nd = ctp->info.srcnd;
		io_openfd.i.info.pid = ctp->info.pid;
		io_openfd.i.info.coid = msg->i.id;

		if(MsgSendnc(fd, &io_openfd.i, sizeof io_openfd.i, 0, 0) == -1) {
			ConnectDetach_r(fd);
			proc_thread_pool_reserve_done();
			return errno;
		}

		if(_devctl(fd, DCMD_CHR_TCSETSID, &sep->leader, sizeof sep->leader, 0) == -1) {
			status = errno;
			close(fd);
			proc_thread_pool_reserve_done();
			return status;
		}
/* unreachable code
		if(sep->fd != -1) {
			close(sep->fd);
		}
*/		
		sep->fd = fd;
		sep->pgrp = sep->leader;	// Posix.1 7.1.1.3
		proc_thread_pool_reserve_done();
		return EOK;
	}		

	case PROCMGR_SESSION_SETSID:
		if(!(prp = proc_lock_pid(ctp->info.pid))) {
			return EL2HLT;
		}

		// Must not already be a session or process group leader
		if((prp->flags & _NTO_PF_SLEADER) || prp->pgrp == prp->pid) {
			return proc_error(EPERM, prp);
		}

		//TODO: There must be no other processes that have this pid as their group PR 1653

		//
		// We need to use _ksmalloc() here because this structure
		// will be _sfree()'d by the kernel later on.  Using _ksmalloc()
		// insures that memory is taken from the correct free list.
		//
		if(!(sep = _ksmalloc(sizeof *sep))) {
			return proc_error(ENOMEM, prp);
		}
		memset(sep, 0, sizeof(*sep));
	
		sep->links = 1;
		sep->leader = prp->pid;
		sep->pgrp = sep->fd = -1;
		if(atomic_sub_value(&prp->session->links, 1) == 1) {
			// This should only happen if the session leader died...
			_ksfree(prp->session, sizeof *prp->session);
		}
		prp->flags |= _NTO_PF_SLEADER;
		prp->session = sep;
		ctp->status = prp->pgrp = prp->pid;

		proc_unlock(prp);
		return EOK;

	case PROCMGR_SESSION_SETPGRP: {
		PROCESS					*prp2;
		int						ret;
		pid_t					leader;

		if(!proc_isaccess(0, &ctp->info)) {
			break;
		}
		if(!(prp = proc_lock_pid(msg->i.sid)) ||
				!(prp->flags & _NTO_PF_SLEADER) ||
				!(sep = prp->session) ||
				sep->fd < 0) {
			return proc_error(EINVAL, prp);
		}

		ret = EOK;
		prp2 = NULL;
		leader = sep->leader;
		if(msg->i.sid == msg->i.id) {
			sep->pgrp = prp->pgrp;
			proc_unlock(prp);
		} else {
			proc_unlock(prp);
			// Don't access "sep" pointer from above past the unlock
			if(!(prp2 = proc_lock_pid(msg->i.id)) || prp2->pid != prp2->pgrp || prp2->session == NULL) {
				// Not sure if this should return EINVAL or EPERM
				ret = EINVAL;
			} else if(prp2->session->leader != leader) {
				ret = EPERM;
			} else {
				// Should we check here that prp2->parent->pid == msg->i.sid??
				prp2->session->pgrp = prp2->pgrp;
			}
		}
		if(prp2 != NULL)
			proc_unlock(prp2);
		return ret;
	}

	case PROCMGR_SESSION_SIGNAL_PID:
		if(!proc_isaccess(0, &ctp->info)) {
			break;
		}
		if(!(prp = proc_lock_pid(msg->i.sid))) {
			return proc_error(EINVAL, prp);
		}
		if(sigismember(&prp->sig_ignore, msg->i.id)) {	// @@@ Need to check sigblock as well
			return proc_error(EIO, prp);
		}
		SignalKill(ND_LOCAL_NODE, -prp->pgrp, 0, msg->i.id, SI_USER, 0);
		return proc_error(EOK, prp);

	case PROCMGR_SESSION_SIGNAL_PGRP:
		if(!proc_isaccess(0, &ctp->info)) {
			break;
		}
		if(!(prp = proc_lock_pid(msg->i.sid)) || !(sep = prp->session) || prp->pid != sep->leader) {
			return proc_error(EINVAL, prp);
		}
		SignalKill(ND_LOCAL_NODE, -sep->pgrp, 0, msg->i.id, SI_USER, 0);
		return proc_error(EOK, prp);

	case PROCMGR_SESSION_SIGNAL_LEADER:
		if(!proc_isaccess(0, &ctp->info)) {
			break;
		}
		if(!(prp = proc_lock_pid(msg->i.sid)) ||
				!(prp->flags & _NTO_PF_SLEADER) ||
				!(sep = prp->session) ||
				sep->fd < 0) {
			return proc_error(EINVAL, prp);
		}
		SignalKill(ND_LOCAL_NODE, sep->leader, 0, msg->i.id, SI_USER, 0);
		return proc_error(EOK, prp);
	default:
		break;
	}

	return ENOSYS;
}

__SRCVERSION("procmgr_session.c $Rev: 169209 $");
