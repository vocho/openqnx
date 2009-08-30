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


static void
kerext_process_bind(void *data) {
	PROCESS	*prp;
	pid_t	pid = *(pid_t *)data;
	THREAD	*act = actives[KERNCPU];

	// Verify the target process and thread exists.
	if(pid <= 0) {
		prp = NULL;
	} else if((prp = lookup_pid(pid)) == NULL) {
		kererr(act, ESRCH);
		return;
	}

	lock_kernel();

#ifndef NDEBUG
	if(prp != NULL) {
		if(act->aspace_prp != NULL) crash();
		if(prp->pid != pid) crash();
	}
	if(act != actives[KERNCPU]) {
		crash();
	}

	{
		PROCESS	*child = act->process;

		if((child->pid != SYSMGR_PID) && !(child->flags & _NTO_PF_VFORKED)) {
			crash();
		}
	}
#endif

	// If pid >= 0, we're setting it, otherwise querying
	if(pid >= 0) {
		act->aspace_prp = prp;
	} else {
		prp = act->aspace_prp;
	}

	SETKSTATUS(act, prp ? prp->pid : 0);
}

int
ProcessBind(pid_t pid) {
	return(__Ring0(kerext_process_bind, &pid));
}

__SRCVERSION("kerext_bind.c $Rev: 153052 $");
