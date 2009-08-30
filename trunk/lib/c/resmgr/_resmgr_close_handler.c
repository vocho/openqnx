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




#include <stdlib.h>
#include <atomic.h>
#include <errno.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include "resmgr.h"

void _resmgr_close_handler(resmgr_context_t *ctp, struct binding *binding) {

	if(atomic_sub_value(&binding->count, 1) == 1) {
		_resmgr_func_t	func;

		if((func = _resmgr_io_func(binding->funcs, _IO_RSVD_CLOSE_OCB))) {
			int							id;

			id = ctp->rcvid;
			ctp->rcvid = 0;
			(void)func(ctp, 0, binding->ocb);
			ctp->rcvid = id;
		}
		free(binding);
	}
}

__SRCVERSION("_resmgr_close_handler.c $Rev: 153052 $");
