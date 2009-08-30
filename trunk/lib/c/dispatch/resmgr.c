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
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <sys/pathmgr.h>
#include <sys/dispatch.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "dispatch.h"
#include "resmgr.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#define MSG_MAX_SIZE	(sizeof(struct _io_connect_link_reply) + sizeof(struct _io_connect_entry) * SYMLOOP_MAX + PATH_MAX + 1)

int resmgr_attach(dispatch_t *dpp, resmgr_attr_t *attr, const char *path, enum _file_type file_type,
		unsigned flags, const resmgr_connect_funcs_t *connect_funcs,
		const resmgr_io_funcs_t *io_funcs, void *handle) {
	_resmgr_control 		*ctrl;
	resmgr_attr_t			null_attr;
	int						rc;

	if(!attr) {
		memset(attr = &null_attr, 0, sizeof null_attr);
	}

	if(!_DPP(dpp)->resmgr_ctrl) {
		message_attr_t			msg_attr;

		if((ctrl = malloc(sizeof *ctrl)) == NULL) {
			errno = ENOMEM;
			return -1;
		}
		memset(ctrl, 0, sizeof *ctrl);

		if((rc = pthread_mutex_init(&ctrl->mutex, 0)) != EOK) {
			free(ctrl);;
			errno = rc;
			return -1;
		}

		memset(&msg_attr, 0, sizeof msg_attr);
		msg_attr.flags = MSG_FLAG_TYPE_RESMGR | ((attr->flags & RESMGR_FLAG_CROSS_ENDIAN) ? MSG_FLAG_CROSS_ENDIAN : 0);

		ctrl->flags = (attr->flags & RESMGR_FLAG_CROSS_ENDIAN) ? _RESMGR_CROSS_ENDIAN : 0;
		ctrl->nparts_max = max(attr->nparts_max, 1);
		ctrl->msg_max_size = max(attr->msg_max_size, max(MSG_MAX_SIZE, sizeof(resmgr_iomsgs_t)));

		ctrl->context_size = sizeof(resmgr_context_t) + ctrl->nparts_max * sizeof(((resmgr_context_t *)0)->iov[0]) + ctrl->msg_max_size;

		if(_dispatch_attach(dpp, ctrl, DISPATCH_RESMGR) == -1) {
			pthread_mutex_destroy(&ctrl->mutex);
			free(ctrl);
			return -1;
		}

		// Attach message type as well
		if(message_attach(dpp, &msg_attr, _IO_BASE, _IO_MAX, _resmgr_msg_handler, (void *)NULL) == -1) {
			return -1;
		}

		msg_attr.flags |= MSG_FLAG_TYPE_PULSE;

		// Attach resmgr pulse types.
		if(message_attach(dpp, &msg_attr, _PULSE_CODE_DISCONNECT, _PULSE_CODE_DISCONNECT, _resmgr_msg_handler, (void *)NULL) == -1) {
			return -1;
		}

		if(message_attach(dpp, &msg_attr, _PULSE_CODE_UNBLOCK, _PULSE_CODE_UNBLOCK, _resmgr_msg_handler, (void *)NULL) == -1) {
			return -1;
		}

	} else {
		int						newsize, newcsize, newiov;

		ctrl = dpp->resmgr_ctrl;

		newiov = max(attr->nparts_max, ctrl->nparts_max);
		newsize = max(ctrl->msg_max_size, max(attr->msg_max_size, max(MSG_MAX_SIZE, sizeof(resmgr_iomsgs_t))));

		newcsize = sizeof(resmgr_context_t) + newiov * sizeof(((resmgr_context_t *)0)->iov[0]) + newsize;

		if((dpp->flags & _DISPATCH_CONTEXT_ALLOCED) && (newcsize > ctrl->context_size || newsize > ctrl->msg_max_size)) {
			errno = EINVAL;
			return -1;
		} else {
			ctrl->flags |= (attr->flags & RESMGR_FLAG_CROSS_ENDIAN) ? _RESMGR_CROSS_ENDIAN : 0;
			ctrl->nparts_max = newiov;
			ctrl->context_size = newcsize;
			ctrl->msg_max_size = newsize;
		}

		if(_dispatch_set_contextsize(dpp, DISPATCH_RESMGR) == -1) {
			errno = EINVAL;
			return -1;
		}
	}

	/*
	 * We allow a resmgr_attach with a NULL path, just to set up the internals
	 */
	if(path || (attr->flags & RESMGR_FLAG_ATTACH_LOCAL) || (flags & _RESMGR_FLAG_FTYPEONLY)) {
		struct link				*linkl;

		if(!(linkl = _resmgr_link_alloc())) {
			errno = ENOMEM;
			return -1;
		}

		if(attr->other_func && !ctrl->other_func) {
			linkl->flags |= _RESMGR_LINK_OTHERFUNC;
			ctrl->other_func = attr->other_func;
			_DPP(dpp)->other_func = (void *)attr->other_func;
		}

		linkl->connect_funcs = connect_funcs;
		linkl->io_funcs = io_funcs;
		linkl->handle = handle;
		if(attr->flags & RESMGR_FLAG_ATTACH_LOCAL) {
			linkl->link_id = -1;
		} else {
			if((linkl->link_id = pathmgr_link(path, 0, 0, _DPP(dpp)->chid, linkl->id, file_type, flags & _RESMGR_FLAG_MASK)) == -1) {
				if(linkl->flags & _RESMGR_LINK_OTHERFUNC) {
					dpp->other_func = NULL;
				}
				_resmgr_link_free(linkl->id, _RESMGR_DETACH_ALL);
				return -1;
			}
		}
		linkl->flags &= ~_RESMGR_LINK_HALFOPEN;
		return linkl->id;
	}
	return 0;
}

resmgr_context_t *resmgr_context_alloc(dispatch_t *dpp) {
	return (resmgr_context_t *)dispatch_context_alloc(dpp);	
}

void resmgr_context_free(resmgr_context_t *ctp) {
	free(ctp);
}

resmgr_context_t *resmgr_block(resmgr_context_t *ctp) {
	// @@ Check if context size has changed, is so, realloc
	ctp->id = -1;
	ctp->info.msglen = 0;

again:
	ctp->rcvid = MsgReceive(ctp->dpp->chid, ctp->msg, ctp->msg_max_size, &ctp->info);
	// Doing a network transaction and not all the message was send, so get the rest...
	if(ctp->rcvid > 0 && ctp->info.srcmsglen > ctp->info.msglen && ctp->info.msglen < ctp->msg_max_size) {
		int						n;

		if((n = MsgRead_r(ctp->rcvid, (char *)ctp->msg + ctp->info.msglen,
				ctp->msg_max_size - ctp->info.msglen, ctp->info.msglen)) < 0) {
			MsgError(ctp->rcvid, -n);
			goto again;
		}
		ctp->info.msglen += n;
	}

	return ctp;
}

int resmgr_handler(resmgr_context_t *ctp) {
	_resmgr_handler(ctp);
	return 0;
}

void resmgr_unblock(resmgr_context_t *ctp) {
	_message_unblock((dispatch_context_t *)ctp);
}

/*
 * This gets called from the message interface if we have both resmgr and 
 * message types registered.
 */
int _resmgr_msg_handler(message_context_t *ctp, int code, unsigned flags, void *handle) {
	_resmgr_handler((resmgr_context_t *)ctp);
	return 0;
}


__SRCVERSION("resmgr.c $Rev: 159268 $");
