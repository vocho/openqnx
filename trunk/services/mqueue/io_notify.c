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
io_notify(resmgr_context_t *ctp, io_notify_t *msg, struct ocb *ocb) {
	MQDEV				*dev = ocb->ocb.attr;
	int					status, n, trig, notifycnts[3] = {1, 1, 1};

	// Calculate which sources are statisfied (immediate trigger).
	trig = 0;
	if(dev->mq_attr.mq_curmsgs) {
		trig |= _NOTIFY_COND_INPUT;
	}
	if(dev->mq_attr.mq_maxmsg - dev->mq_attr.mq_curmsgs > 0) {
		trig |= _NOTIFY_COND_OUTPUT;
	}

	//
	// Process the notify request.
	// "n" will be set to 1 if a notify entry is armed.
	//
	status = iofunc_notify(ctp, msg, &dev->notify[0], trig, notifycnts, &n);

	/* 
		Tie the ocb which attached the nofication.  On close we'll need
		this to determine if the close() should remove the notification

		PR 1207
	 */
	if (n)
		ocb->notify_attached++;
	else
		ocb->notify_attached--;

	//
	// Since the messages can't change under our feet we don't need to
	// recheck them like we did in chario which has async input from
	// an interrupt handler.
	//

	return status;
}

__SRCVERSION("io_notify.c $Rev: 153052 $");
