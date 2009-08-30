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




#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <share.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_openfd(resmgr_context_t *ctp, io_openfd_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	io_open_t	io_open;
	int			status;

	memset(&io_open, 0x00, sizeof io_open);
	io_open.connect.ioflag = msg->i.ioflag & (_IO_FLAG_MASK | O_SETFLAG | O_TRUNC);
	io_open.connect.sflag = msg->i.sflag;
	io_open.connect.access = ocb->ioflag & _IO_FLAG_MASK;

	/* We want to compare the operational permissions (R,W,RW) and not open attributes */
	if((io_open.connect.ioflag & io_open.connect.access) != (io_open.connect.ioflag & _IO_FLAG_MASK)) {
		return EACCES;
	}

	if((status = _iofunc_open(ctp, &io_open, attr, 0, 0)) != EOK) {
		return status;
	}
	msg->i.ioflag = io_open.connect.ioflag;

	return iofunc_ocb_attach(ctp, &io_open, 0, attr, _resmgr_iofuncs(ctp, &msg->i.info));
}

__SRCVERSION("iofunc_openfd.c $Rev: 153052 $");
