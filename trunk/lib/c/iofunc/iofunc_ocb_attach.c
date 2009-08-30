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




#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <share.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_ocb_attach(resmgr_context_t *ctp, io_open_t *msg, iofunc_ocb_t *ocb,
				iofunc_attr_t *attr, const resmgr_io_funcs_t *io_funcs) {
	unsigned						sflag;
	iofunc_ocb_t					*ocb2 = 0;
	register iofunc_funcs_t			*funcs = 0;

	if(!ocb) {
		register iofunc_mount_t			*mountp;
		register iofunc_ocb_t			*(*ocb_calloc)(resmgr_context_t *ctp, iofunc_attr_t *attr);
		
		if((mountp = attr->mount) && (funcs = mountp->funcs) &&
				funcs->nfuncs >= (offsetof(iofunc_funcs_t, ocb_calloc) / sizeof(void *)) &&
				(ocb_calloc = funcs->ocb_calloc)) {
			ocb = ocb_calloc(ctp, attr);
		} else {
			ocb = iofunc_ocb_calloc(ctp, attr);
		}
		if(!(ocb2 = ocb)) {
			return errno;
		}
	}

	ocb->ioflag = msg->connect.ioflag & (~_IO_FLAG_MASK | msg->connect.access);
	ocb->sflag = msg->connect.sflag;
	ocb->flags = 0;
	ocb->offset = 0;
	ocb->attr = attr;
		
	if(resmgr_open_bind(ctp, ocb, io_funcs) == -1) {
		int				status = errno;				

		if(ocb2) {
			register void					(*ocb_free)(iofunc_ocb_t *ocb);

			if(funcs && funcs->nfuncs >= (offsetof(iofunc_funcs_t, ocb_free) / sizeof(void *)) &&
					(ocb_free = funcs->ocb_free)) {
				ocb_free(ocb);
			} else {
				iofunc_ocb_free(ocb);
			}
		}
		return status;
	}

	if(ocb->ioflag & _IO_FLAG_WR) {
		attr->wcount++;
	}

	if(ocb->ioflag & _IO_FLAG_RD) {
		attr->rcount++;
	}

	sflag = ocb->sflag & SH_MASK;

	if(sflag == SH_DENYRW || sflag == SH_DENYRD) {
		attr->rlocks++;
	}

	if(sflag == SH_DENYRW || sflag == SH_DENYWR) {
		attr->wlocks++;
	}

	attr->count++;

	return EOK;
}

__SRCVERSION("iofunc_ocb_attach.c $Rev: 153052 $");
