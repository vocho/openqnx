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




#include <errno.h>
#include <sys/iofunc.h>

int iofunc_time_update(iofunc_attr_t *attr) {
	iofunc_mount_t			*mountp;

	if((mountp = attr->mount)) {
		if(mountp->flags & _MOUNT_READONLY) {
			return EOK;
		}
		if((mountp->flags & _MOUNT_NOATIME) && !(attr->flags & 
				(IOFUNC_ATTR_DIRTY_MASK | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME))) {
			return EOK;
		}
	}
	if(attr->flags & (IOFUNC_ATTR_ATIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME)) {
		time_t					clockl = time(0);
		
		if(attr->flags & IOFUNC_ATTR_ATIME) {
			attr->atime = clockl;
		}
		if(attr->flags & IOFUNC_ATTR_MTIME) {
			attr->mtime = clockl;
		}
		if(attr->flags & IOFUNC_ATTR_CTIME) {
			attr->ctime = clockl;
		}
		attr->flags &= ~(IOFUNC_ATTR_ATIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME);
		attr->flags |= IOFUNC_ATTR_DIRTY_TIME;
	}
	return EOK;
}

__SRCVERSION("iofunc_time_update.c $Rev: 153052 $");
