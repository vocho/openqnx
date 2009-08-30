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

int iofunc_write_default(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb) {
	int					status;

	// Check we have the correct perms
	if((status = iofunc_write_verify(ctp, msg, ocb, 0)) != EOK) {
		return status;
	}

	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		break;
	default:
		// We don't handle this xtype, return error
		return ENOSYS;
	}

	if(_IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes)) {
		// Only flag modification and change time for update if more than zero bytes where written
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
	}

	return EOK;
}

__SRCVERSION("iofunc_write_default.c $Rev: 153052 $");
