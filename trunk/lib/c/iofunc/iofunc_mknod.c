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

int iofunc_mknod(resmgr_context_t *ctp, io_mknod_t *msg, iofunc_attr_t *attr, iofunc_attr_t *dattr, struct _client_info *info) {
	struct _client_info			buff;
	int							status;

	if(!dattr) {
		return EBADFSYS;
	}

	// Get client info if no info passed in
	if(!info && (status = iofunc_client_info(ctp, msg->connect.ioflag, info = &buff)) != EOK) {
		return status;
	}

	// Check for write perm in parent
	if((status = _iofunc_create(ctp, dattr, &msg->connect.mode, dattr->mount, info)) != EOK) {
		return status;
	}

	// initialize the attr
	if(attr) {
		iofunc_attr_init(attr, msg->connect.mode, dattr, info);
		attr->rdev = msg->connect.file_type;
	}

	// Done
	return EOK;
}

__SRCVERSION("iofunc_mknod.c $Rev: 153052 $");
