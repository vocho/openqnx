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




#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <atomic.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_dup_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg) {
	int							n, id;
	struct binding				*binding;
	_resmgr_func_t				func;

	if((binding = (struct binding *)_resmgr_handle(&msg->dup.i.info, 0, _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {
		return EBADF;
	}

	atomic_add (&binding->count, 1);
	_resmgr_handle(&msg->dup.i.info, 0, _RESMGR_HANDLE_UNLOCK);

	n = _resmgr_access(ctp, &msg->dup.i.info, 0, msg->dup.i.key, msg, offsetof(struct _io_dup, key));
	if(n != EOK) {
		_resmgr_close_handler(ctp, binding);
		return n;
	}

	ctp->id = binding->id;
	if((func = _resmgr_io_func(binding->funcs, _IO_DUP))) {
		switch((unsigned)(n = func(ctp, msg, binding->ocb))) {
		case _RESMGR_DEFAULT:
		case EOK:
			break;
		default:
			_resmgr_close_handler(ctp, binding);
			return n;
		}
	}


	if(_resmgr_handle(&ctp->info, binding, _RESMGR_HANDLE_SET) != (void *)-1) {
		return n;
	}
	n = errno;

	if(func && (func = _resmgr_io_func(binding->funcs, _IO_CLOSE))) {
		id = ctp->rcvid;
		ctp->rcvid = 0;
		(void)func(ctp, msg, binding->ocb);
		ctp->rcvid = id;
	}

	_resmgr_close_handler(ctp, binding);
	return n;
}

__SRCVERSION("_resmgr_dup_handler.c $Rev: 153052 $");
