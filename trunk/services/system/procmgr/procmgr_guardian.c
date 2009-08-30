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

int procmgr_msg_guardian(resmgr_context_t *ctp, proc_guardian_t *msg) {
	PROCESS				*prp, *child;
	pid_t				pid;

	if(!(prp = proc_lock_pid(ctp->info.pid))) {
		return EL2HLT;
	}

	pid = prp->guardian ? prp->guardian->pid : 0;

	if(msg->i.pid) {
		if(pid == -1) {
			prp->guardian = 0;
		} else  {
			for(child = prp->child; child; child = child->sibling) {
				if(msg->i.pid == child->pid) {
					prp->guardian = child;		
					break;
				}
			}
			if(!child) {
				return proc_error(ECHILD, prp);
			}
		}
	}

	proc_unlock(prp);

	msg->o.zero1 = 0;
	msg->o.pid = pid;
	SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o);
	return _RESMGR_NPARTS(1);
}

__SRCVERSION("procmgr_guardian.c $Rev: 153052 $");
