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
io_unblock(resmgr_context_t *ctp, io_pulse_t *msg, struct ocb *ocb) {
	MQDEV				*dev = ocb->ocb.attr;
	MQWAIT				*wp, *wp_prev;
	struct _msg_info	info;
	
	// Check if rcvid is still valid and still has an unblock request pending.
	if(MsgInfo(ctp->rcvid, &info) == -1 || !(info.flags & _NTO_MI_UNBLOCK_REQ)) {
		return _RESMGR_NOREPLY;
	}

	// Remove from blocked readers.
	wp_prev = (MQWAIT *) &dev->waiting_read;
	for(; wp = wp_prev->next ; wp_prev = wp) {
		if(wp->rcvid == ctp->rcvid) {
			wp_prev->next = wp->next;
			MemchunkFree(memchunk, wp);
			return EINTR;
		}
	}

	// Remove from blocked writers.
	wp_prev = (MQWAIT *) &dev->waiting_write;
	for(; wp = wp_prev->next ; wp_prev = wp) {
		if(wp->rcvid == ctp->rcvid) {
			wp_prev->next = wp->next;
			MemchunkFree(memchunk, wp);
			return EINTR;
		}
	}

	return _RESMGR_NOREPLY;
}

__SRCVERSION("io_unblock.c $Rev: 153052 $");
