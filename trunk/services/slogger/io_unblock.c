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
io_unblock(resmgr_context_t *ctp, io_pulse_t *msg, iofunc_ocb_t *ocb) {
	struct waiting	*wap, *wap_prev;
	struct slogdev	*trp;

	// Remove from blocked readers.
	trp = (struct slogdev *) ocb->attr;
	wap_prev = (struct waiting *) &trp->waiting;
	for(; (wap = wap_prev->next) ; wap_prev = wap)
		if(wap->rcvid == ctp->rcvid) {
			wap_prev->next = wap->next;
			free(wap);
			return(EINTR);
		}

	return(_RESMGR_NOREPLY);
}

__SRCVERSION("io_unblock.c $Rev: 153052 $");
