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
#include <errno.h>
#include <gulliver.h>
#include <sys/dispatch.h>
#include "resmgr.h"
#include "dispatch/dispatch.h"

void _resmgr_handler(resmgr_context_t *ctp) {
	int								n;
	resmgr_iomsgs_t					*msg;
	dispatch_t						*dpp;
	struct pulse_func				*p;
	struct binding					*binding;
	unsigned						flags;
	int (*func)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *ocb);

	msg = ctp->msg;
	dpp = ctp->dpp;
	n = _RESMGR_DEFAULT;
	ctp->status = 0;
	if(ctp->msg_max_size == 0) {
		// If max_msg_size == 0, dpp isn't actually a dispatch_t structure.
		// Instead, it's an old resmgr_ctrl_t allocated by _resmgr_thread
		// (the 'reserved' field in the old resmgr_context_t is at the same 
		// location as 'msg_max_size' in a the new one and we set that
		// to zero in _resmgr_thread). Anyhow, we can't look at the flags
		// field to see if it's a cross endian thing since that isn't there
		// in the old code. Just set flags to zero since the old
		// code can't possibly be cross-endian aware.

		// Note that 'dpp->other_func' is being referenced below, but that's
		// OK because the 'other_func' field was cleverly placed at the
		// same offset in both the dispatch_t structure and the old
		// resmgr_ctrl_t.
		flags = 0;
	} else {
		flags = dpp->resmgr_ctrl->flags;
	}
	if(flags & _RESMGR_CROSS_ENDIAN) resmgr_endian_context(ctp, -1, S_IFREG, 0);
	if(ctp->rcvid != -1) {
		if(ctp->rcvid == 0) {
			if(msg->type == _PULSE_TYPE && msg->pulse.subtype == _PULSE_SUBTYPE) {
				struct _pulse pulse;

				switch(msg->pulse.code) {
				case _PULSE_CODE_DISCONNECT:
					(void)_resmgr_disconnect_handler(ctp, msg, msg->pulse.scoid);
					break;

				case _PULSE_CODE_UNBLOCK:
					pulse = msg->pulse;
					n = _resmgr_unblock_handler(ctp, msg, msg->pulse.value.sival_int);
					if((unsigned)n == _RESMGR_DEFAULT) {
						msg->pulse = pulse;
						ctp->rcvid = 0;
						if(dpp->other_func) {
							n = dpp->other_func(ctp, msg);
						}
					}
					break;

				default:
					_mutex_lock(&_resmgr_io_table.mutex);
					for(p = _resmgr_pulse_list; p; p = p->next) {
						if(p->code == msg->pulse.code) {
							_mutex_unlock(&_resmgr_io_table.mutex);
							p->func(ctp, p->code, msg->pulse.value, p->handle);
							break;
						}
					}
					if(!p) {
						_mutex_unlock(&_resmgr_io_table.mutex);
						if(dpp->other_func) {
							(void)dpp->other_func(ctp, msg);
						}
					}
					break;
				}
				if((unsigned)n == _RESMGR_DEFAULT) {
					return;
				}
			}
		} else {
			if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
				if(!(flags & _RESMGR_CROSS_ENDIAN) ||
						resmgr_endian(ctp, msg) != EOK) {
					MsgError(ctp->rcvid, EENDIAN);
					return;
				}
			}
			switch(msg->type) {
			case _IO_CONNECT:
				n = _resmgr_connect_handler(ctp, msg);
				break;

			case _IO_DUP:
				n = _resmgr_dup_handler(ctp, msg);
				break;

			case _IO_MMAP:
				n = _resmgr_mmap_handler(ctp, msg);
				break;

			case _IO_OPENFD:
				n = _resmgr_openfd_handler(ctp, msg);
				break;

			case _IO_CLOSE:
				if((binding = (struct binding *)_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_REMOVE_LOCK)) == (void *)-1) {
					n = EBADF;
					break;
				}
				if((func = _resmgr_io_func(binding->funcs, _IO_CLOSE))) {
					ctp->offset = 0;
					ctp->size = ctp->info.msglen;
					ctp->id = binding->id;
					n = func(ctp, msg, binding->ocb);
				}
				else {
					n = EOK;
				}

				_resmgr_close_handler(ctp, binding);

				break;
				
			case _IO_NOTIFY:
				if((msg->notify.i.flags & _NOTIFY_COND_EXTEN) &&
				    (msg->notify.i.action & _NOTIFY_ACTION_POLL)) { /* POLL or POLLARM */
					n = _resmgr_notify_handler(ctp, msg);
					break;
				}
				/* Fallthrough */

			default:
				if(msg->type >= _IO_BASE && msg->type <= _IO_MAX) {
					if((binding = (struct binding *)_resmgr_handle(&ctp->info, 0, _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {
						n = EBADF;
					} else {
						n = _resmgr_io_handler(ctp, msg, binding);
					}
				} else {
					n = _RESMGR_DEFAULT;
				}
				break;
			}
		}
	}

	if((unsigned)n == _RESMGR_DEFAULT && dpp->other_func) {
		n = dpp->other_func(ctp, msg);
	}

	switch((unsigned)n) {
	case _RESMGR_NOREPLY:
		break;

	case _RESMGR_DEFAULT:
		n = ENOSYS;
		/* Fall through */
	default:
		if(n <= 0) {
			(void)resmgr_msgreplyv(ctp, ctp->iov, -n);
		} else {
			MsgError(ctp->rcvid,  n);
		}
	}
}


__SRCVERSION("_resmgr_handler.c $Rev: 155997 $");
