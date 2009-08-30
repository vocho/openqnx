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
#include <sys/memmsg.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <atomic.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_mmap_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg) {
	int							n;
	struct binding				*binding;

	if(ctp->info.pid != SYSMGR_PID) {
		return ENOSYS;
	}

	if((binding = (struct binding *)_resmgr_handle(&msg->mmap.i.info, 0, _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {
		return EBADF;
	}

	atomic_add(&binding->count, 1);
	_resmgr_handle(&msg->mmap.i.info, 0, _RESMGR_HANDLE_UNLOCK);

	_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_FIND_LOCK);

	n = _resmgr_io_handler(ctp, msg, binding);

	_resmgr_close_handler(ctp, binding);

	return n;
}

__SRCVERSION("_resmgr_mmap_handler.c $Rev: 153052 $");
