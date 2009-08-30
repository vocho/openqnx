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




#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_disconnect_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, int scoid) {
	struct binding				*binding;

	ctp->info.scoid = scoid;
	while((binding = (struct binding *)_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_DISCONNECT_LOCK)) != (void *)-1) {
		ctp->info.msglen = sizeof msg->close;
		msg->close.i.type = _IO_CLOSE;
		msg->close.i.combine_len = sizeof msg->close.i;
		(void)_resmgr_io_handler(ctp, msg, binding);
	}
	ConnectDetach(scoid);
	return _RESMGR_NOREPLY;
}

__SRCVERSION("_resmgr_disconnect_handler.c $Rev: 153052 $");
