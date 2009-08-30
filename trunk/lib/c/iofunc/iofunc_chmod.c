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

int iofunc_chmod(resmgr_context_t *ctp, io_chmod_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	int						status;
	mode_t					mode;
	struct _client_info		info;

	// Get the client credentials
	if((status = iofunc_client_info(ctp, ocb->ioflag, &info)) != EOK) {
		return status;
	}

	// Check for owner or appropriate privileges
	if((status = iofunc_check_access(ctp, attr, S_ISUID, &info)) != EOK) {
		return status;
	}

	// Is it a read only filesystem?
	if(attr->mount && (attr->mount->flags & _MOUNT_READONLY)) {
		if (!(attr->flags & IOFUNC_ATTR_SYNTHETIC))
			return EROFS;
	}

	// Update the mode
	mode = (attr->mode & ~(S_IPERMS | S_ISUID | S_ISGID | S_ISVTX)) |
			(msg->i.mode & (S_IPERMS | S_ISUID | S_ISGID | S_ISVTX));

	// Knock down the sticky bit if not a directory and not enough perms
	if((mode & S_ISVTX) && !S_ISDIR(mode) && iofunc_check_access(ctp, attr, 0, &info) != EOK) {
		mode &= ~S_ISVTX;
	}

	// Knock down set-group-id bit as per POSIX 1003.1 section 5.6.4.2
	if((mode & S_ISGID) && !S_ISDIR(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) &&
			iofunc_check_access(ctp, attr, S_ISGID, &info) != EOK) {
		mode &= ~S_ISGID;
	}

	// If mode is changed, mark it dirty
	if(mode != attr->mode) {
		attr->flags |= IOFUNC_ATTR_DIRTY_MODE;
		attr->mode = mode;
	}

	// Mark ctime for update
	attr->flags |= (IOFUNC_ATTR_CTIME | IOFUNC_ATTR_DIRTY_TIME);
	return EOK;
}

__SRCVERSION("iofunc_chmod.c $Rev: 153052 $");
