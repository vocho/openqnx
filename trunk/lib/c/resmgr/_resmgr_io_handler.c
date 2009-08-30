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
#include <atomic.h>
#include <sys/resmgr.h>
#include "resmgr.h"

static void _resmgr_unlock(resmgr_context_t *ctp, struct binding *binding) {
	_resmgr_func_t	func;

	if((func = _resmgr_io_func(binding->funcs, _IO_RSVD_UNLOCK_OCB))) {
		(void)func(ctp, 0, binding->ocb);
	}
}

int _resmgr_io_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, struct binding *binding) {
	int							n;
	int							locked;

	/*
	 * Although all threads who would get a reference to binding via same coid are
	 * blocked, should still atomic this since another thread could get a reference
	 * via duped coid.
	 */
	atomic_add(&binding->count, 1);
	_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_UNLOCK);

	locked = 0;
	ctp->id = binding->id;
	n = EBADMSG;
	while((ctp->offset = (char *)msg - (char *)(ctp->msg)) < ctp->info.msglen) {
		int						rcvid;
		int						combine;
		_resmgr_func_t					func;

		ctp->size = ctp->info.msglen - ctp->offset;
		rcvid = ctp->rcvid;
		combine = 0;
		if(msg->combine.combine_len & _IO_COMBINE_FLAG) {
			ctp->size = combine = msg->combine.combine_len & ~_IO_COMBINE_FLAG;
			ctp->rcvid = 0;
		}

		func = _resmgr_io_func(binding->funcs, msg->type);

		switch(msg->type) {
		case _IO_CLOSE:
			/*
			 * Pass in the binding we're working on as a hint.
			 * If it doesn't match, it was already removed (and
			 * possible replaced) while we were unlocked.
			 */
			if(_resmgr_handle(&ctp->info, binding, _RESMGR_HANDLE_REMOVE_LOCK) != (void *)-1) {
				if(locked) {
					_resmgr_unlock(ctp, binding);
					locked = 0;
				}
				n = func ? func(ctp, msg, binding->ocb) : EOK;
				atomic_sub(&binding->count, 1);
			}
			combine = 0;
			break;

		default:
			if(func) {
				_resmgr_func_t 	lock;

				if(!locked && (lock = _resmgr_io_func(binding->funcs, _IO_RSVD_LOCK_OCB))) {
					int		rcvid2 = ctp->rcvid;

					ctp->rcvid = rcvid;
					if((n = lock(ctp, 0, binding->ocb)) != EOK) {
						break;
					}
					ctp->rcvid = rcvid2;
					locked = 1;
				}
				n = func(ctp, msg, binding->ocb);
			} else {
				n = ENOSYS;
			}
			break;
		}

		ctp->rcvid = rcvid;
		if(combine == 0 || n != EOK) {
			break;
		}

		ctp->status = 0;
		msg = (resmgr_iomsgs_t *)((char *)msg + combine);
		if((n = resmgr_endian(ctp, msg)) != EOK) {
			break;
		}
		n = EBADMSG;
	}

	if(locked) {
		_resmgr_unlock(ctp, binding);
	}
	_resmgr_close_handler(ctp, binding);
	return n;
}

__SRCVERSION("_resmgr_io_handler.c $Rev: 153052 $");
