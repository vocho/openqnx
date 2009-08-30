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




#define _FILE_OFFSET_BITS		64
#define _IOFUNC_OFFSET_BITS		64
#include <unistd.h>
#include <errno.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_lseek(resmgr_context_t *ctp, io_lseek_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	off64_t		offset;

	offset = msg->i.offset;

	switch(msg->i.whence) {
	case SEEK_SET:
		break;

	case SEEK_CUR:
		offset += ocb->offset;
		break;

	case SEEK_END:
		offset += ocb->attr->nbytes;
		break;

	default:
		return EINVAL;
	}

	if(offset < 0) {
		return EINVAL;
	}

	if (IS32BIT(attr, ocb->ioflag) && offset > LONG_MAX)
		return(EOVERFLOW);

	ocb->offset = offset;

	if(msg->i.combine_len & _IO_COMBINE_FLAG) {
		return EOK;
	}

	msg->o = offset;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

__SRCVERSION("iofunc_lseek.c $Rev: 153052 $");
