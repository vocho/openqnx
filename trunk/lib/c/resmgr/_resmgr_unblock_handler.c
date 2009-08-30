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
#include <atomic.h>
#include <unistd.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_unblock_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, int rcvid) {
	struct binding				*binding;
	int 							(*func)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *ocb);
	int ret = _RESMGR_DEFAULT;
	struct _msg_info *infop = &ctp->info;

	if(MsgInfo(ctp->rcvid = rcvid, &ctp->info) == -1) {
		return _RESMGR_DEFAULT;
	}

	if((binding = (struct binding *)_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {
		SETIOV(ctp->iov + 0, msg, offsetof(struct _io_openfd, reserved2));
		if(MsgReadv(ctp->rcvid, ctp->iov + 0, 1, 0) == -1) {
			return _RESMGR_DEFAULT;
		}
		
		if (msg->type != _IO_OPENFD || (binding = (struct binding *)_resmgr_handle(&msg->openfd.i.info, 0, _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {

			if(msg->type != _IO_CONNECT) {
				return _RESMGR_DEFAULT;
			}
			msg->connect.subtype = _IO_CONNECT_RSVD_UNBLOCK;
			return _resmgr_connect_handler(ctp, msg);
		}
		infop=&msg->openfd.i.info;
	}

	atomic_add(&binding->count, 1);
	_resmgr_handle(infop, 0, _RESMGR_HANDLE_UNLOCK);

	if((func = _resmgr_io_func(binding->funcs, _IO_RSVD_UNBLOCK))) {
		ret = func(ctp, msg, binding->ocb);
	}

	_resmgr_close_handler(ctp, binding);

	return ret;

}

__SRCVERSION("_resmgr_unblock_handler.c $Rev: 153052 $");
