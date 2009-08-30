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
#include <sys/dispatch.h>
#include "resmgr.h"
#include "dispatch/dispatch.h"

int resmgr_msg_again(resmgr_context_t *ctp, int rcvid) {
	if(MsgInfo(rcvid, &ctp->info) == -1) {
		return -1;
	}

	if((ctp->info.msglen = MsgRead(ctp->rcvid = rcvid, ctp->msg, ctp->msg_max_size, 0)) == -1) {
		return -1;
	}

	(void)MsgCurrent(rcvid);

	_resmgr_handler(ctp);

	return 0;
}

__SRCVERSION("resmgr_again.c $Rev: 153052 $");
