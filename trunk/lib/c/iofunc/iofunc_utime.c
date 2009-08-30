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

int iofunc_utime(resmgr_context_t *ctp, io_utime_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	int						status;
	struct _client_info		info;

	// Get the client credentials
	if((status = iofunc_client_info(ctp, ocb->ioflag, &info)) != EOK) {
		return status;
	}

	// Check for owner or appropriate privileges (if current also allow writers)
	if((status = iofunc_check_access(ctp, attr, msg->i.cur_flag ? S_ISUID | S_IWRITE : S_ISUID, &info)) != EOK) {
		return status;
	}

	// Is it a read only filesystem?
	if(attr->mount && (attr->mount->flags & _MOUNT_READONLY)) {
		if (!(attr->flags & IOFUNC_ATTR_SYNTHETIC))
			return EROFS;
	}

	if(msg->i.cur_flag) {
		attr->flags |= (IOFUNC_ATTR_ATIME | IOFUNC_ATTR_MTIME);
	} else {
		attr->atime = msg->i.times.actime;
		attr->mtime = msg->i.times.modtime;
		attr->flags &= ~(IOFUNC_ATTR_ATIME | IOFUNC_ATTR_MTIME);
	}

	// Mark ctime for update
	attr->flags |= (IOFUNC_ATTR_CTIME | IOFUNC_ATTR_DIRTY_TIME);

	return iofunc_time_update(attr);
}

__SRCVERSION("iofunc_utime.c $Rev: 153052 $");
