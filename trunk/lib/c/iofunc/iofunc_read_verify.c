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
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_read_verify(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb, int *nonblock) {

	if((ocb->ioflag & _IO_FLAG_RD) == 0) {
		return EBADF;
	}

	if (IS32BIT(ocb->attr, ocb->ioflag) && S_ISREG(ocb->attr->mode) && msg->i.nbytes != 0) {
	off64_t		off;

		off = ((msg->i.xtype & _IO_XTYPE_MASK) == _IO_XTYPE_OFFSET) ? ((struct _xtype_offset *)(&msg->i + 1))->offset : ocb->offset;
		if (off >= LONG_MAX)
			return(EOVERFLOW);
		if (off + msg->i.nbytes > LONG_MAX)
			msg->i.nbytes = LONG_MAX - off;
	}

	if(nonblock) {
		*nonblock = _iofunc_isnonblock(ocb->ioflag, msg->i.xtype);
	}

	return EOK;
}

__SRCVERSION("iofunc_read_verify.c $Rev: 153052 $");
