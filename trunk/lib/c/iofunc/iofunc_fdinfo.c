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
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_fdinfo(resmgr_context_t *ctp, iofunc_ocb_t *ocb, iofunc_attr_t *attr, struct _fdinfo *info) {
	if(!attr) {
		attr = ocb->attr;
	}
	info->mode = attr->mode;
	info->ioflag = ocb->ioflag;
	info->offset = ocb->offset;
	info->size = attr->nbytes;
	info->sflag = ocb->sflag;
	info->count = attr->count;
	info->rcount = attr->rcount;
	info->wcount = attr->wcount;
	info->rlocks = attr->rlocks;
	info->wlocks = attr->wlocks;
	memset(&info->zero, 0x00, sizeof info->zero);
	info->flags = 0;
	if(PTR_VALUE(attr->lock_list, iofunc_lock_list_t)) {
		info->flags |= _FDINFO_LOCKS;
	}
	if(attr->mmap_list) {
		info->flags |= _FDINFO_MMAPS;
	}
	return EOK;
}

__SRCVERSION("iofunc_fdinfo.c $Rev: 153052 $");
