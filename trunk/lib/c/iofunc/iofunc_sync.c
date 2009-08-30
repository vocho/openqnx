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
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_sync(resmgr_context_t *ctp, iofunc_ocb_t *ocb, int ioflag) {
	iofunc_attr_t				*attr;
	iofunc_mount_t				*mountp;

	// Not on a readonly filesystem
	if((mountp = (attr = ocb->attr)->mount) && (mountp->flags & _MOUNT_READONLY)) {
		return 0;
	}

	// If called from read, only do a sync if O_RSYNC is set...
	if((ioflag & _IO_FLAG_MASK) != _IO_FLAG_RD || (ocb->ioflag & O_RSYNC)) {

		// if called from read or write, use the OCB flags as well
		if(ioflag & _IO_FLAG_MASK) {
			ioflag |= ocb->ioflag;
		}

		// O_SYNC is a superset of O_DSYNC, check it first
		if(ioflag & O_SYNC) {
			// Even if _MOUNT_NOATIME is set, set DIRTY_TIME flag when O_SYNC is requested
			if(attr->flags & (IOFUNC_ATTR_ATIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME)) {
				attr->flags |= IOFUNC_ATTR_DIRTY_TIME;
			}
			// Return non-zero if anything is dirty
			if(attr->flags & IOFUNC_ATTR_DIRTY_MASK) {
				return O_SYNC;
			}
		} else if(ioflag & O_DSYNC) {
			// Posix seems to imply that only time can be ignored....
			if(attr->flags & (IOFUNC_ATTR_DIRTY_MASK & ~IOFUNC_ATTR_DIRTY_TIME)) {
				return O_DSYNC;
			}
		}
	}

	return 0;
}

__SRCVERSION("iofunc_sync.c $Rev: 153052 $");
