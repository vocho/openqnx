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




#define _FILE_OFFSET_BITS       64
#define _IOFUNC_OFFSET_BITS     64
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <share.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int _iofunc_open(resmgr_context_t *ctp, io_open_t *msg, iofunc_attr_t *attr, iofunc_attr_t *dattr, struct _client_info *info) {
	unsigned					ioflag, sflag;
	mode_t						mode;
	int							status;
	iofunc_mount_t				*mountp;

	// Get ioflag
	ioflag = msg->connect.ioflag & (~_IO_FLAG_MASK | msg->connect.access);

	// Get mount pointer
	mountp = (dattr && dattr->mount) ? dattr->mount : (attr ? attr->mount : 0);

	// If sync flags are set, and sync io is not supported, return an error
	if((ioflag & (O_SYNC | O_DSYNC | O_RSYNC)) && (!mountp || (mountp->conf & IOFUNC_PC_SYNC_IO) == 0)) {
		return EINVAL;
	}

	// If "dattr" is not null, "attr" is being created in directory "dattr".
	if(dattr) {
		// If not opened for creat, return an error
		if(!(ioflag & O_CREAT)) {
			return ENOENT;
		}

		// Check for write perm in parent
		if((status = _iofunc_create(ctp, dattr, &msg->connect.mode, mountp, info)) != EOK) {
			return status;
		}

		// Turn off the sticky bit
		msg->connect.mode &= ~S_ISVTX;

		// If no format defined, make it a regular file.
		if((msg->connect.mode & S_IFMT) == 0) {
			msg->connect.mode |= S_IFREG;
		}
		// Otherwise for symlinks, set all access permissions
		else if(S_ISLNK(msg->connect.mode)) {
			msg->connect.mode |= S_IPERMS;
		}

		// initialize the attr
		if(attr) {
			iofunc_attr_init(attr, msg->connect.mode, dattr, info);
		}

		// Done
		return EOK;
	}

	// Check share access
	sflag = msg->connect.sflag & SH_MASK;
	if(	((ioflag & _IO_FLAG_RD) && attr->rlocks) ||
		((ioflag & _IO_FLAG_WR) && attr->wlocks) ||
		((sflag == SH_DENYRW || sflag == SH_DENYRD) && attr->rcount) ||
		((sflag == SH_DENYRW || sflag == SH_DENYWR) && attr->wcount)) {
		return EBUSY;
	}

	// Check for existing file
	if((ioflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
		return EEXIST;
	}

	// O_TRUNC should also have write access, else silently ignore it
	if((ioflag & (_IO_FLAG_WR | O_TRUNC)) == O_TRUNC) {
		ioflag = msg->connect.ioflag &= ~O_TRUNC;
	}

	// Is it opened for updating?
	if(ioflag & _IO_FLAG_WR) {
		// Don't allow writing a directory
		if(S_ISDIR(attr->mode)) {
			return EISDIR;
		}

		// Must be a writable file system
		if(mountp && (mountp->flags & _MOUNT_READONLY)) {
			return EROFS;
		}
	}

	// Get permision check mode
	if(info != NULL) {
		mode = 0;
		if(ioflag & _IO_FLAG_RD) {
			mode |= S_IREAD;
		}
		if(ioflag & _IO_FLAG_WR) {
			mode |= S_IWRITE;
		}
		// If reading or writing, check permissions
		if(mode != 0 && (status = iofunc_check_access(ctp, attr, mode, info)) != EOK) {
			return status;
		}
	}

	// check largefile semantics
	if(IS32BIT(attr, ioflag) && S_ISREG(attr->mode) && attr->nbytes > LONG_MAX) {
		return EOVERFLOW;
	}

	// If otrunc set, mark file for time updates (unless FIFO)
	if(ioflag & O_TRUNC && !S_ISFIFO(attr->mode)) {
		attr->flags |= IOFUNC_ATTR_DIRTY_TIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
	}

	// It can be opened
	return EOK;
}

__SRCVERSION("_iofunc_open.c $Rev: 153052 $");
