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
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_space_verify(resmgr_context_t *ctp, io_space_t *msg, iofunc_ocb_t *ocb, int *nonblock) {
	iofunc_attr_t		*attr;

	// must be open for write
	if((ocb->ioflag & _IO_FLAG_WR) == 0) {
		return EBADF;
	}

	attr = ocb->attr;

	// read-only attribute may have been set since open() verifications
	if(attr->mount && (attr->mount->flags & _MOUNT_READONLY)) {
		return EROFS;
	}

	// space is not allowed on directories (checked in iofunc_open)
	if(S_ISDIR(attr->mode)) {
		return EISDIR;
	}

	// catch non-largefile ftruncate() usage
	if (IS32BIT(attr, ocb->ioflag) && S_ISREG(attr->mode)) {
		if (msg != NULL && msg->i.whence == SEEK_SET && msg->i.start + msg->i.len > LONG_MAX)
			return(EFBIG);
	}

	// knock down set-id-bits if executable
	if((attr->mode & (S_ISUID | S_ISGID | S_ISVTX)) && (attr->mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
		int						status;
		struct _client_info		info;

		// Get the client credentials
		if((status = iofunc_client_info(ctp, ocb->ioflag, &info)) != EOK) {
			return status;
		}

		// Check for owner or appropriate privileges
		if((status = iofunc_check_access(ctp, attr, 0, &info)) != EOK) {
			attr->mode &= ~(S_ISUID | S_ISGID | S_ISVTX);
			attr->flags |= IOFUNC_ATTR_DIRTY_MODE;
		}
	}

	if(nonblock) {
		*nonblock = _iofunc_isnonblock(ocb->ioflag, 0);
	}

	return EOK;
}

__SRCVERSION("iofunc_space_verify.c $Rev: 153052 $");
