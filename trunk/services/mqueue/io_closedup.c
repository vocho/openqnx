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





#include "externs.h"


static void
unblock_one(resmgr_context_t *ctp, struct ocb *ocb, MQWAIT **waiting) {
	MQWAIT				*wp;
	MQWAIT				**owner;

	owner = waiting;
	for( ;; ) {
		wp = *owner;
		if(wp == NULL) break;
		if((wp->scoid == ctp->info.scoid) && (wp->coid == ctp->info.coid)) {
			MsgError(wp->rcvid, EBADF);
			*owner = wp->next;
			MemchunkFree(memchunk, wp);
		} else {
			owner = &wp->next;
		}
	}
}

void
unblock_all(resmgr_context_t *ctp, struct ocb *ocb) {
	MQDEV		*dev = ocb->ocb.attr;

	unblock_one(ctp, ocb, &dev->waiting_read);
	unblock_one(ctp, ocb, &dev->waiting_write);
}

int
io_closedup(resmgr_context_t *ctp, io_close_t *msg, struct ocb *ocb) {
#ifndef MQUEUE_1_THREAD
	MQDEV		*dev = ocb->ocb.attr;
#endif

#ifndef MQUEUE_1_THREAD
	iofunc_attr_lock(&dev->attr);
#endif

	unblock_all(ctp, ocb);

#ifndef MQUEUE_1_THREAD
	iofunc_attr_unlock(&dev->attr);
#endif

	return EOK;
}

__SRCVERSION("io_closedup.c $Rev: 153052 $");
