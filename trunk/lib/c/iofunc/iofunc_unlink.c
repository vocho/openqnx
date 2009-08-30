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
#include <errno.h>
#include <share.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_unlink(resmgr_context_t *ctp, io_unlink_t *msg, iofunc_attr_t *attr, iofunc_attr_t *dattr, struct _client_info *info) {
	struct _client_info			buff;
	int							status;
	iofunc_mount_t				*mountp;

	// If no attr or dattr is not a directory there is a problem
	if(!attr || (dattr && !S_ISDIR(dattr->mode))) {
		return EBADFSYS;
	}

	// Get mount pointer
	mountp = attr->mount ? attr->mount : (dattr ? dattr->mount : 0);

	if(msg) {
		// Get client info if no info passed in
		if(!info && (status = iofunc_client_info(ctp, msg->connect.ioflag, info = &buff)) != EOK) {
			return status;
		}

		// Was it a "rmdir"?
		if(S_ISDIR(msg->connect.mode)) {
			// Only a directory can be removed with rmdir
			if(!S_ISDIR(attr->mode)) {
				return ENOTDIR;
			}

			// ".." is not allowed to be removed
			if(msg->connect.eflag & _IO_CONNECT_EFLAG_DOTDOT) {
				return EEXIST;
			}

			// "." is not allowed to be removed
			if(msg->connect.eflag & _IO_CONNECT_EFLAG_DOT) {
				return EINVAL;
			}

			// Directory must be empty (2 is itself and ".")
			if(attr->nlink > 2) {
				return ENOTEMPTY;
			}
		} else if(S_ISDIR(attr->mode)) {
			// Does file system allow unlink of directories?
			if(!mountp || (mountp->conf & IOFUNC_PC_LINK_DIR) == 0) {
				return EPERM;
			}

			// Only root can unlink a directory
			if((status = iofunc_check_access(ctp, attr, 0, info)) != EOK) {
				return status;
			}
		}
	}

	if(!dattr || (dattr->mode & S_ISVTX)) {
		// If sticky is set on directory then owner of file or
		// owner of directory or write permision on file may remove.
		if(iofunc_check_access(ctp, attr, S_IWRITE, info) != EOK) {
			if(iofunc_check_access(ctp, attr, S_ISUID, info) != EOK) {
				if(!dattr || iofunc_check_access(ctp, dattr, S_ISUID, info) != EOK) {
					return EPERM;
				}
			}
		}
	} else {
		// If not sticky, just need write permision on directory.
		if((status = iofunc_check_access(ctp, dattr, S_IWRITE, info)) != EOK) {
			return status;
		}
	}

	// Would link count become invalid?
	if(attr->nlink < (S_ISDIR(attr->mode) ? 2 : 1)) {
		return EBADFSYS;
	}

	// Must be a writable file system
	if(mountp && (mountp->flags & _MOUNT_READONLY)) {
		return EROFS;
	}

	// We have permision, now do it
	if(dattr) {
		// If removing a directory decrement parent link count (for "..")
		if(S_ISDIR(attr->mode)) {
			if(dattr->nlink < 1) {
				return EBADFSYS;
			}
			dattr->nlink--;
			dattr->flags |= IOFUNC_ATTR_DIRTY_NLINK;
		}

		// mark parent time flags
		dattr->flags |= IOFUNC_ATTR_DIRTY_TIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
	}

	// decrement the link count
	attr->nlink--;
	if(S_ISDIR(attr->mode)) {
		// unlinking a directory imlies unlinking "." so decrement again
		attr->nlink--;
	}

	// mark file time flags and link dirty
	attr->flags |= IOFUNC_ATTR_DIRTY_NLINK | IOFUNC_ATTR_DIRTY_TIME | IOFUNC_ATTR_CTIME;

	return EOK;
}

__SRCVERSION("iofunc_unlink.c $Rev: 153052 $");
