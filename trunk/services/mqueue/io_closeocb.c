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


int
io_closeocb(resmgr_context_t *ctp, void *reserved, struct ocb *ocb) {
	MQDEV		*dev = ocb->ocb.attr;

#ifndef MQUEUE_1_THREAD
	iofunc_attr_lock(&dev->attr);
#endif

	unblock_all(ctp, ocb);

	// Cancel any io_notify in effect.

	/* 
		PR 1207 - if *our* ocb attached notification remove it.
		Stops others with open()/close() on this mq from removing
		the notification prematurely
	*/
	if (ocb->notify_attached)
	{
		iofunc_notify_remove(ctp, &dev->notify[0]);
		ocb->notify_attached=0; /* sanity */
	}

	// Deliver and free a possible close msg (set via a devctl).
	if(ocb->closemsg) {
		io_write(ctp, NULL, ocb);
		free(ocb->closemsg);
		ocb->closemsg = NULL;
	}

	iofunc_close_ocb(ctp, &ocb->ocb, &dev->attr);

#ifndef MQUEUE_1_THREAD
	iofunc_attr_unlock(&dev->attr);
#endif

	// Was the device unlinked while it was in use?
	if(dev->attr.count == 0  &&  dev->attr.nlink == 0) {
		resmgr_detach(dpp, dev->id, _RESMGR_DETACH_ALL);
		delete_msgs(dev);
		MemchunkFree(memchunk, dev);
	}

	return EOK;
}

__SRCVERSION("io_closeocb.c $Rev: 153052 $");
