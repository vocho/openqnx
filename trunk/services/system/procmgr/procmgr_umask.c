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

int procmgr_umask(resmgr_context_t *ctp, proc_umask_t *msg) {
	PROCESS				*prp;
	unsigned			old_umask;

	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->i.subtype);
		ENDIAN_SWAP32(&msg->i.umask);
		ENDIAN_SWAP32(&msg->i.pid);
	}
	switch(msg->i.subtype) {
	case _PROC_UMASK_GET:
	case _PROC_UMASK_SET:
		if(!(prp = proc_lock_pid(msg->i.pid ? msg->i.pid : ctp->info.pid))) {
			return ESRCH;
		}
		old_umask = prp->umask;
		if(msg->i.subtype == _PROC_UMASK_SET) {
			if(msg->i.pid != 0 && !proc_isaccess(prp, &ctp->info)) {
				return proc_error(EPERM, prp);
			}
			prp->umask = msg->i.umask;
		}
		proc_unlock(prp);
		break;
	default:
		return ENOSYS;
	}
	msg->o.zero1 = 0;
	msg->o.umask = old_umask;
	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP32(&msg->o.umask);
	}
	SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o);
	return _RESMGR_NPARTS(1);
}

__SRCVERSION("procmgr_umask.c $Rev: 153052 $");
