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

int iofunc_open(resmgr_context_t *ctp, io_open_t *msg, iofunc_attr_t *attr, iofunc_attr_t *dattr, struct _client_info *info) {
	struct _client_info			buff;
	int							status;

	// Get credential info
	if(!info && (status = iofunc_client_info(ctp, msg->connect.ioflag, info = &buff)) != EOK) {
		return status;
	}

	return _iofunc_open(ctp, msg, attr, dattr, info);
}

__SRCVERSION("iofunc_open.c $Rev: 153052 $");
