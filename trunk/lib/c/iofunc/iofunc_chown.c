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




#include <unistd.h>
#include <errno.h>
#include <sys/iofunc.h>

int iofunc_chown(resmgr_context_t *ctp, io_chown_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	int						status;
	struct _client_info		info;
	int						restricted;
	iofunc_mount_t			*mountp;

	restricted = (mountp = attr->mount) ? (mountp->conf & IOFUNC_PC_CHOWN_RESTRICTED) != 0 : !0;

	// Get the client credentials
	if((status = iofunc_client_info(ctp, ocb->ioflag, &info)) != EOK) {
		return status;
	}

	// passed in values of -1 means no change!
	if(msg->i.uid == -1) {
		msg->i.uid = attr->uid;
	}
	if(msg->i.gid == -1) {
		msg->i.gid = attr->gid;
	}

	// Check for owner or appropriate privileges
	if((status = iofunc_check_access(ctp, attr, restricted ? 0 : S_ISUID, &info)) != EOK) {
		unsigned						n;

		if(!restricted || attr->uid != msg->i.uid || info.cred.euid != attr->uid) {
			return status;
		}

		for(n = 0; n < info.cred.ngroups; n++) {
			if(info.cred.grouplist[n] == msg->i.gid) {
				break;
			}
		}
		if(n >= info.cred.ngroups && info.cred.egid != msg->i.gid) {
			return status;
		}
	}

	// Is it a read only filesystem?
	if(mountp && mountp->flags & _MOUNT_READONLY) {
		if (!(attr->flags & IOFUNC_ATTR_SYNTHETIC))
			return EROFS;
	}

	// If not root knock down set-user-id and set-group-id bits as per POSIX 1003.1 section 5.6.5.1
	if(info.cred.euid != 0 && (attr->mode & (S_ISUID | S_ISGID)) && !S_ISDIR(attr->mode) &&
			(attr->mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
		attr->mode &= ~(S_ISUID | S_ISGID);
		attr->flags |= IOFUNC_ATTR_DIRTY_MODE;
	}

	if(msg->i.uid != attr->uid || msg->i.gid != attr->gid) {
		attr->uid = msg->i.uid;
		attr->gid = msg->i.gid;
		attr->flags |= IOFUNC_ATTR_DIRTY_OWNER;
	}

	// Mark ctime for update
	attr->flags |= (IOFUNC_ATTR_CTIME | IOFUNC_ATTR_DIRTY_TIME);
	return EOK;
}

__SRCVERSION("iofunc_chown.c $Rev: 153052 $");
