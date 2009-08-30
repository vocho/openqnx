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

int procmgr_getsetid(resmgr_context_t *ctp, proc_getsetid_t *msg) {
	PROCESS						*prp;
	int							isroot;
	int							*rid, *eid, *sid;
	struct _cred_info			info;
	pid_t						leader;

	if(!(prp = proc_lock_pid(msg->i.pid ? msg->i.pid : ctp->info.pid))) {
		return ESRCH;
	}

	(void)CredGet(prp->pid, &info);

	switch(msg->i.subtype) {
	case _PROC_ID_GETID: 

		// Fill in return parameters before the check below as we
		// need to first unlock prp (to avoid deadlocks)
		msg->o.pgrp = prp->pgrp;
		msg->o.ppid = prp->parent ? prp->parent->pid : PROCMGR_PID;
		msg->o.sid = prp->session ? prp->session->leader : 0;
		msg->o.cred = info;
		SETIOV(ctp->iov + 0, &msg->o, offsetof(struct _proc_getsetid_reply, cred.grouplist)
				+ msg->o.cred.ngroups * sizeof info.grouplist[0]);

		//Only check session match if we are looking up a foreign pid
		//@@@: Should sessionless people be allowed to see each other?
		leader = prp->session ? prp->session->leader : -1;
		proc_unlock(prp);
		prp = NULL;

		if (msg->i.pid && msg->i.pid != ctp->info.pid) {
			PROCESS *prp2;
	
			if(!(prp2 = proc_lock_pid(ctp->info.pid))) {
				return ESRCH;
			}

			if ((prp2->session == NULL) || (prp2->session->leader != leader)) {
				return proc_error(EPERM, prp2);
			}

			proc_unlock(prp2);
		}

		return _RESMGR_NPARTS(1);

	case _PROC_ID_SETGROUPS:
		if(!proc_isaccess(0, &ctp->info)) {
			// Only allow root to change group lists
			return proc_error(EPERM, prp);
		}

		if(msg->i.ngroups < 0 || msg->i.ngroups > sizeof info.grouplist / sizeof info.grouplist[0]) {
			return proc_error(EINVAL, prp);
		}

		// Get the current credential and update the grouplist.
		info.ngroups = msg->i.ngroups;
		memcpy(info.grouplist, (&msg->i + 1), msg->i.ngroups * sizeof info.grouplist[0]);
		CredSet(prp->pid, &info);
		break;

	case _PROC_ID_SETUID:
	case _PROC_ID_SETEUID:
	case _PROC_ID_SETREUID:
		// Changing user parameters
		rid = &info.ruid;
		eid = &info.euid;
		sid = &info.suid;
        goto setid1;

	case _PROC_ID_SETGID:
	case _PROC_ID_SETEGID:
	case _PROC_ID_SETREGID:
		// Changing group parameters
		rid = &info.rgid;
		eid = &info.egid;
		sid = &info.sgid;

setid1:
		isroot = proc_isaccess(0, &ctp->info);
		if(!isroot && prp->pid != ctp->info.pid) {
			// Only allow root to change someone elses perms
			return proc_error(EPERM, prp);
		}

		switch(msg->i.subtype) {
		case _PROC_ID_SETUID:
		case _PROC_ID_SETGID:
			// set?id acts like sete?id if not root
			if(isroot) {
				msg->i.rid = msg->i.eid;
				goto setid2;
			}
			/* Fall through */

		case _PROC_ID_SETEUID:
		case _PROC_ID_SETEGID:
			// sete?id doesn't change the real id
			msg->i.rid = -1;

			if(isroot) {
				// If root, don't allow change saved-id
				sid = 0;
			}
			/* Fall through */

		case _PROC_ID_SETREUID:
		case _PROC_ID_SETREGID:
			if(!isroot) {
				// If not root and trying to change eid, it must be equal to rid or sid
				if(msg->i.eid != -1 && msg->i.eid != *rid && msg->i.eid != *sid) {
					return proc_error(EPERM, prp);
				}

				// If not root and trying to change rid, it must be equal to eid
				if(msg->i.rid != -1 && msg->i.rid != *eid) {
					return proc_error(EPERM, prp);
				}
			}

	setid2:
			// change the effective id
			if(msg->i.eid != -1) {
				*eid = msg->i.eid;
			}
	
			// change the real id
			if(msg->i.rid != -1) {
				*rid = msg->i.rid;
			}
	
			// If changeing real, or changing effective to other that read, set saved to effective
			if(sid && (msg->i.rid != -1 || (msg->i.eid != -1 && msg->i.eid != *rid))) {
				*sid = *eid;
			}

			CredSet(prp->pid, &info);
			break;
		default:
			break;
		}
		break;

	default:
		return proc_error(ENOSYS, prp);
	}

	return proc_error(EOK, prp);
}

__SRCVERSION("procmgr_getsetid.c $Rev: 153052 $");
