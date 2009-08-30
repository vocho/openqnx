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

int procmgr_setpgid(resmgr_context_t *ctp, proc_setpgid_t *msg) {
	PROCESS							*prp1, *prp2, *prp3;
	int								ret;

	// If pgid is less than zero return EINVAL
	if(msg->i.pgid < 0) {
		return EINVAL;
	}

#if 0  /* For full session support */
	// callers controlling terminal must support job control
	if(!prp1->session || !(prp1->session->flags & _NTO_SF_NOJOBCTL)) {
		return proc_error(EINVAL, prp1);
	}
#endif

	// Note that we have to be careful in here to maintain the lock order
	// used elsewhere in proc - child then parent else we can deadlock.
	// See PR 23567.
	ret = EOK;
	prp3 = NULL;
	prp1 = NULL;
	if(msg->i.pid == 0 || msg->i.pid == ctp->info.pid) {
		if(!(prp1 = proc_lock_pid(ctp->info.pid))) {
			return EL2HLT;
		}
		prp2 = prp1;
	} else {
		// Lock prp2 first. 
		if(!(prp2 = proc_lock_pid(msg->i.pid)) || (prp2->parent == NULL)) {
			ret = ESRCH; goto out_unlock;
		}

		// If pid is not a child of the calling process return ESRCH
		if(prp2->parent->pid != ctp->info.pid) {
			ret = ESRCH; goto out_unlock;
		}

		// If pid is a child that has execed, return EACCES 
		//@@@ This catches spawns _and_ exec's
		if(!(prp2->flags & _NTO_PF_FORKED)) {
			ret = EACCES; goto out_unlock;
		}

		// prp1 is the parent so now we can lock him
		if(!(prp1 = proc_lock_pid(ctp->info.pid))) {
			ret = EL2HLT; goto out_unlock;
		}

	}

	// If pid is a session leader or not in the same session return EPERM
	if((prp2->flags & _NTO_PF_SLEADER) || prp1->session != prp2->session) {
		ret = EPERM; goto out_unlock;
	}

	// If session is null for either prp1 or prp2 return ESRCH
	if((prp1->session == NULL) || (prp2->session == NULL)) {
		ret = ESRCH; goto out_unlock;
	}

	// Note that we now snapshot some values in case we need to 
	// lock prp3 (in which case we will need to unlock prp1/prp2).
	// Unfortunately, prp3 is not necessarily an ancestor 
	// of prp1/prp2 but may be a peer. Therefore, we cannot hold
	// all three locks at once.

	// if pgid is zero, set process group to indicated pid
	if(msg->i.pgid == 0) {
		prp2->pgrp = prp2->pid;
	} else {
		// If process group is same as calling pid, set process group
		if(msg->i.pgid != prp1->pid) {
			pid_t	leader1, leader3, pid2;

			leader1 = prp1->session->leader;
			pid2 = prp2->pid;

			if(msg->i.pgid == prp2->pid) {
				prp3 = prp2;
				leader3 = prp3->session->leader;
			} else {
				// Careful.. unlock prp1/prp2 first
				proc_unlock(prp1);
				if(prp2 != prp1) proc_unlock(prp2);
				prp1 = prp2 = NULL;

				//If this is not a process group at all, don't set it
				if(!(prp3 = proc_lock_pid(msg->i.pgid)) ||
				    prp3->pgrp != msg->i.pgid) {
					ret = EPERM; goto out_unlock;
				}
				// Defensive
				if(prp3->session == NULL) {
					ret = ESRCH; goto out_unlock;
				}

				leader3 = prp3->session->leader;
				proc_unlock(prp3);
				prp3 = NULL;

				// Now we need to relock prp2
				if(!(prp2 = proc_lock_pid(pid2))) {
					ret = ESRCH; goto out_unlock;
				}
				// This leaves prp1/prp3 as null and prp2 set. The code
				// below should unlock properly
			}

			// If no process group leader is not in same session return EPERM
			if(leader1 != leader3) {
				ret = EPERM; goto out_unlock;
			}
		}
		prp2->pgrp = msg->i.pgid;
	}
out_unlock:
	if(prp3 != NULL && prp3 != prp2 && prp3 != prp1)
		proc_unlock(prp3);
	if(prp2 != NULL && prp2 != prp1)
		proc_unlock(prp2);
	return proc_error(ret, prp1);
}

__SRCVERSION("procmgr_setpgid.c $Rev: 153052 $");
