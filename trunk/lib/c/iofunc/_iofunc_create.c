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
#include <fcntl.h>
#include <share.h>
#include <errno.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int _iofunc_create(resmgr_context_t *ctp, iofunc_attr_t *attr, mode_t *mode, iofunc_mount_t *mountp, struct _client_info *info) {
	int							status;

	if(!attr || !S_ISDIR(attr->mode) || !info) {
		return EBADFSYS;
	}

	// Let other manager handle creations
	if(mountp || (mountp = attr->mount)) {
		if(mountp->flags & _MOUNT_NOCREAT) {
			return ENOSYS;
		}
	}

	// Parent must not be write locked
	if(attr->wlocks) {
		return EBUSY;
	}

	// Check for write perm in parent
	if((status = iofunc_check_access(ctp, attr, S_IWRITE, info)) != EOK) {
		return status;
	}

	// Must be a writable file system
	if(mountp && mountp->flags & _MOUNT_READONLY) {
		return EROFS;
	}

	if(mode) {
		// If not a member of the current groups, knock down the 
		if((*mode & S_ISGID) && (attr->mode & S_ISGID) &&
				iofunc_check_access(ctp, attr, S_ISGID, info) != EOK) {
			*mode &= ~S_ISGID;
		}
	}
	
	// mark parent time flags
	attr->flags |= IOFUNC_ATTR_DIRTY_TIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	// Done
	return EOK;
}

__SRCVERSION("_iofunc_create.c $Rev: 153052 $");
