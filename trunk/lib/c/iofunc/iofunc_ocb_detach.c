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
#include <share.h>
#include <malloc.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_ocb_detach(resmgr_context_t *ctp, iofunc_ocb_t *ocb) {
	unsigned						sflag;
	unsigned						flag;
	iofunc_attr_t					*attr = ocb->attr;

	// Make sure times are current
	(void)iofunc_time_update(attr);

	// Clean up any mmaped files
	if(ocb->flags & IOFUNC_OCB_MMAP) {
		iofunc_mmap_list_t			*p, **pp;

		for(pp = &attr->mmap_list; (p = *pp); pp = &p->next) {
			if(p->ocb == ocb) {
				*pp = p->next;
				free(p);
				break;
			}
		}
	}


	flag = 0;

	if(ocb->ioflag & _IO_FLAG_WR) {
		if(--attr->wcount == 0) {
			flag |= IOFUNC_OCB_LAST_WRITER;
		}
	}

	if(ocb->ioflag & _IO_FLAG_RD) {
		if(--attr->rcount == 0) {
			flag |= IOFUNC_OCB_LAST_READER;
		}
	}

	sflag = ocb->sflag & SH_MASK;

	if(sflag == SH_DENYRW || sflag == SH_DENYRD) {
		if(--attr->rlocks == 0) {
			flag |= IOFUNC_OCB_LAST_RDLOCK;
		}
	}

	if(sflag == SH_DENYRW || sflag == SH_DENYWR) {
		if(--attr->wlocks == 0) {
			flag |= IOFUNC_OCB_LAST_WRLOCK;
		}
	}

	if(--attr->count == 0) {
		flag |= IOFUNC_OCB_LAST_INUSE;
	}

	return flag;
}

__SRCVERSION("iofunc_ocb_detach.c $Rev: 153052 $");
