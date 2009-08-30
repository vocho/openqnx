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
#include <errno.h>
#include <pthread.h>
#include <sys/resmgr.h>
#include "resmgr.h"

#define ALIGNMSG(b, o, i) (void *)((char *)b + (((o) + (i) + _QNX_MSG_ALIGN - 1) & ~(_QNX_MSG_ALIGN - 1)))

typedef int(*_resmgr_connect_func_t)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *handle, void *extra);

int _resmgr_connect_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg) {
	struct link						*link;
	const resmgr_connect_funcs_t	*funcs;

	if(!(link = _resmgr_link_query(msg->open.connect.handle, 1))) {
		return ENOENT;
	}

	ctp->id = link->id;

	if(!(funcs = link->connect_funcs)) {
		_resmgr_link_return(link, 1);
		return _RESMGR_DEFAULT;
	}

	// MsgKeyData verification goes here!!!

	ctp->offset = 0;
	switch(msg->open.connect.subtype) {
	case _IO_CONNECT_COMBINE:
	case _IO_CONNECT_COMBINE_CLOSE: {
		int							n;
		int							rcvid;
		int							subtype;
		struct binding				*binding;
		resmgr_iomsgs_t				*combined;

		if(!funcs->open) {
			_resmgr_link_return(link, 1);
			return ENOSYS;
		}

		// Fake up an open message
		subtype = msg->open.connect.subtype;
		ctp->size = offsetof(struct _io_connect, path) + msg->open.connect.path_len;

		pthread_setspecific(_resmgr_thread_key, link);
		n = funcs->open(ctp, (io_open_t *)&msg->open.connect, link->handle, 0);

		if((link = pthread_getspecific(_resmgr_thread_key))) {
			pthread_setspecific(_resmgr_thread_key, NULL);
			_resmgr_link_return(link, 1);
		}

		if(n != EOK || (ctp->status & _IO_CONNECT_RET_FLAG)) {
			return n;
		}
		if((binding = (struct binding *)_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {
			return EBADF;
		}
		atomic_add (&binding->count, 1);

		if(msg->open.connect.extra_len >= sizeof(struct _io_combine)) {
			// Call the combined message passing the opened OCB
			ctp->status = 0;
			combined = (resmgr_iomsgs_t *)ALIGNMSG(msg, offsetof(struct _io_connect, path), msg->open.connect.path_len);
			if((n = resmgr_endian(ctp, combined)) == EOK) {
				n = _resmgr_io_handler(ctp, combined, binding);
				// _resmgr_io_handler unlocks so reacquire 
				if(_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_FIND_LOCK) == (void *)-1) {
					_resmgr_close_handler(ctp, binding);
					return EBADF;
				}
			}
		}

		// Finish the transaction calling the close message, but ignore its return value
		if(subtype == _IO_CONNECT_COMBINE_CLOSE && _resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_FIND) != (void *)-1) {
			int			status = ctp->status;

			rcvid = ctp->rcvid;
			ctp->rcvid = 0;
			msg->close.i.type = _IO_CLOSE;
			msg->close.i.combine_len = sizeof msg->close.i;
			(void)_resmgr_io_handler(ctp, msg, binding);
			ctp->rcvid = rcvid;
			ctp->status = status;
		}
		else
			_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_UNLOCK);

		_resmgr_close_handler(ctp, binding);
		return n;
	}
	default:
		if(msg->connect.subtype - _IO_CONNECT_OPEN < funcs->nfuncs) {
			int status;
			_resmgr_connect_func_t	func;
   	
			if((func = *(((_resmgr_connect_func_t *)(&funcs->open)) + msg->connect.subtype - _IO_CONNECT_OPEN))) {
				void						*extra = ALIGNMSG(msg, offsetof(struct _io_connect, path), msg->open.connect.path_len);

				ctp->size = ctp->info.msglen;

				pthread_setspecific(_resmgr_thread_key, link);
				if(msg->connect.subtype == _IO_CONNECT_LINK && msg->link.connect.extra_type == _IO_CONNECT_EXTRA_LINK) {
					status = _resmgr_link_handler(ctp, msg, link, extra, func);
				}
				else if(msg->connect.subtype == _IO_CONNECT_MOUNT) {
					status = _resmgr_mount_handler(ctp, msg, link, extra, func);
				} else {
					status = func(ctp, msg, link->handle, extra);
				}

				if ((link = pthread_getspecific(_resmgr_thread_key))) {
					pthread_setspecific(_resmgr_thread_key, NULL);
					_resmgr_link_return(link, 1);
				}

				return status;
			}
		}
		_resmgr_link_return(link, 1);
		break;
	}
	return _RESMGR_DEFAULT;
}

__SRCVERSION("_resmgr_connect_handler.c $Rev: 153052 $");
