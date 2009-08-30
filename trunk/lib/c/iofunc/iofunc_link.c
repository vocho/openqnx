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
#include <fcntl.h>
#include <share.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_link(resmgr_context_t *ctp, io_link_t *msg, iofunc_attr_t *attr, iofunc_attr_t *dattr, struct _client_info *info) {
	struct _client_info			buff;
	int							status;
	iofunc_mount_t				*mountp;
	mode_t						old_mode;

	// If no attr or no dattr, there is a problem.
	if(!attr || !dattr) {
		return EBADFSYS;
	}

	mountp = attr->mount ? attr->mount : dattr->mount;

	if(msg) {
		// Get client info if no info passed in
		if(!info && (status = iofunc_client_info(ctp, msg->connect.ioflag, info = &buff)) != EOK) {
			return status;
		}

		if(S_ISDIR(attr->mode)) {
			// Does file system allow link on directories?
			if(!mountp || (mountp->conf & IOFUNC_PC_LINK_DIR) == 0) {
				return EPERM;
			}

			// Only root can unlink a directory
			if((status = iofunc_check_access(ctp, attr, 0, info)) != EOK) {
				return status;
			}
		}
	}

	// save old mode
	old_mode = attr->mode;

	if((status = _iofunc_create(ctp, dattr, &attr->mode, mountp, info)) != EOK) {
		return status;
	}

	// If mode is changed, flag it.
	if(old_mode != attr->mode) {
		attr->flags |= IOFUNC_ATTR_DIRTY_MODE;
	}

	// If linking a directory increment parent link count (for "..")
	if(S_ISDIR(attr->mode)) {
		dattr->nlink++;
		dattr->flags |= IOFUNC_ATTR_DIRTY_NLINK;
	}

	// mark parent time flags
	dattr->flags |= IOFUNC_ATTR_DIRTY_TIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	// increment the link count
	attr->nlink++;
	if(S_ISDIR(attr->mode)) {
		// linking a directory imlies linking "." so increment again
		attr->nlink++;
	}

	// mark file time flags and link dirty
	attr->flags |= IOFUNC_ATTR_DIRTY_NLINK | IOFUNC_ATTR_DIRTY_TIME | IOFUNC_ATTR_CTIME;

	return EOK;
}

__SRCVERSION("iofunc_link.c $Rev: 153052 $");
