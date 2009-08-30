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




#include <errno.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_link_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, struct link *link, struct _msg_info *info,
			int	(*func)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *handle, void *ocb)) {
	struct binding				*binding;
	int							status;

	// Lookup passed in handle
	if((binding = (struct binding *)_resmgr_handle(info, 0, _RESMGR_HANDLE_FIND)) == (void *)-1) {
		return ENXIO;
	}

	// Make sure they are the same device
	if(link->id != binding->id) {
		return ENXIO;
	}

	// Make sure they are from the same process
	if((status = _resmgr_access(ctp, info, 0, 0, 0, 0)) != EOK) {
		return status;
	}

	return func(ctp, msg, link->handle, (io_link_extra_t *)&binding->ocb);
}

__SRCVERSION("_resmgr_link_handler.c $Rev: 153052 $");
