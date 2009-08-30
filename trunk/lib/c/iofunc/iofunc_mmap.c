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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/mman.h>
#include <share.h>
#include "iofunc.h"

// The resource manager will not call out unless the sending process is
// the process manager/memory manager (pid==1)
int iofunc_mmap(resmgr_context_t *ctp, io_mmap_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	iofunc_mmap_list_t			*l;
	int							status;
	unsigned					ioflag;
	register iofunc_mount_t		*mountp;
	register iofunc_funcs_t		*funcs;
	register void				(*ocb_free)(iofunc_ocb_t *ocb);
	register iofunc_ocb_t		*(*ocb_calloc)(resmgr_context_t *ctp, iofunc_attr_t *attr);

	ocb_calloc = iofunc_ocb_calloc;
	ocb_free = iofunc_ocb_free;
	if((mountp = attr->mount) && (funcs = mountp->funcs)) {
		if(funcs->nfuncs >= (offsetof(iofunc_funcs_t, ocb_calloc) / sizeof(void *)) && funcs->ocb_calloc) {
			ocb_calloc = funcs->ocb_calloc;
		}
		if(funcs->nfuncs >= (offsetof(iofunc_funcs_t, ocb_free) / sizeof(void *)) && funcs->ocb_free) {
			ocb_free = funcs->ocb_free;
		}
	}
  
	ioflag = _IO_FLAG_RD;
	if(msg->i.prot & PROT_WRITE) {
		if(mountp && (mountp->flags & _MOUNT_READONLY)) {
			return EROFS;
		}
		ioflag |= _IO_FLAG_WR;
	}

	// If mandatory locks enabled, and locks exist, return an error
	if(PTR_VALUE(attr->lock_list, iofunc_lock_list_t) && (attr->mode & (S_IXGRP | S_ISGID)) == S_ISGID) {
		return EAGAIN;
	}

	if((ocb->ioflag & ioflag) != ioflag) {
		return EACCES;
	}
			
	// find a previous connection if there was one
	for(l = attr->mmap_list; l; l = l->next) {
		if(l->scoid == ctp->info.scoid) {
			break;
		}
	}

	if(l) {
		ocb = l->ocb;
		if((ioflag & _IO_FLAG_WR) && !(ocb->ioflag & _IO_FLAG_WR)) {
			ocb->ioflag |= _IO_FLAG_WR;
			attr->wcount++;
		}
	} else {
		if(!(l = calloc(sizeof *l, 1))) {
			status = ENOMEM;
		} else if(!(ocb = l->ocb = ocb_calloc(ctp, attr))) {
			status = ENOMEM;
		} else {
			ocb->attr = attr;
			ocb->ioflag = ioflag;
			ocb->flags = IOFUNC_OCB_PRIVILEGED | IOFUNC_OCB_MMAP;
			if(resmgr_open_bind(ctp, ocb, _resmgr_iofuncs(ctp, &msg->i.info)) == -1) {
				status = errno;
			} else {
				status = EOK;
				l->coid = ctp->info.coid;
				l->scoid = ctp->info.scoid;
				l->next = attr->mmap_list;
				attr->mmap_list = l;
				attr->count++;
				attr->rcount++;
				if(ioflag & _IO_FLAG_WR) {
					attr->wcount++;
				}
				ocb->sflag = SH_DENYWR;
				attr->wlocks++;
			}
		}
		if(status != EOK) {
			if(l) {
				if(l->ocb) {
					ocb_free(l->ocb);
				}
				free(l);
			}
			return status;
		}
	}

	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.coid = l->coid;

	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

__SRCVERSION("iofunc_mmap.c $Rev: 153052 $");
