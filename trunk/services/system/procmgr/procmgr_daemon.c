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

#include <sys/procmgr.h>
#include "externs.h"
#include "procmgr_internal.h"

int procmgr_msg_daemon(resmgr_context_t *ctp, proc_daemon_t *msg) {
	PROCESS								*prp;
	int									fd;

	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->i.subtype);
		ENDIAN_SWAP32(&msg->i.status);
		ENDIAN_SWAP32(&msg->i.flags);
	}

	if(!(prp = proc_lock_pid(ctp->info.pid))) {
		return ESRCH;
	}

	// First detach from parent returning a status if nessessary
	procmgr_nozombie(prp, msg->i.status);

	// Now detach from session and join proc's session
	// Remember an fd that may need to be closed while not proc_lock()ed
	fd = -1;
	if(prp->flags & _NTO_PF_SLEADER) {
		prp->pgrp = 1; // So we don't drop a SIGHUP on ourselves
		fd = procmgr_sleader_detach(prp);
	}
	if(atomic_sub_value(&prp->session->links, 1) == 1) {
		// This should only happen if the session leader died...
		_ksfree(prp->session, sizeof *prp->session);
	}
	prp->session = sysmgr_prp->session;
	atomic_add(&prp->session->links, 1);
	prp->pgrp = prp->pid;

	// Change directory to root
	if(!(msg->i.flags & PROCMGR_DAEMON_NOCHDIR)) {
		if(prp->cwd != prp->root) {
			NODE	*old = prp->cwd;

			prp->cwd = pathmgr_node_clone(prp->root);
			pathmgr_node_detach(old);
		}
	}

	// Clear the umask
	if(!(msg->i.flags & PROCMGR_DAEMON_KEEPUMASK)) {
		prp->umask = 0;
	}

	// Return the highest used fd so the lib can close them
	ctp->status = prp->fdcons.nentries;
   
	// unlock the process;
	proc_unlock(prp);

	// Free up any fd's if nessessary
	if(fd != -1) {
		if(proc_thread_pool_reserve() != 0) {
			return EAGAIN;
		}
		close(fd);
		proc_thread_pool_reserve_done();
	}

	return EOK;
}

__SRCVERSION("procmgr_daemon.c $Rev: 169209 $");
