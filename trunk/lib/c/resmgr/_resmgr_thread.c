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
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <atomic.h>
#define RESMGR_COMPAT
#include <sys/resmgr.h>
#include "resmgr.h"

static void _thread_cleanup(void *data) {
	resmgr_context_t			*ctp = data;
	resmgr_ctrl_t				*ctrl = ctp->ctrl;

	pthread_mutex_lock(&ctrl->mutex);
	atomic_sub(&ctrl->waiting, 1);
	atomic_sub(&ctrl->created, 1);
	pthread_mutex_unlock(&ctrl->mutex); 
	free(ctp);
}

void *_resmgr_thread(void *data) {
	resmgr_ctrl_t						*ctrl = data;
	resmgr_context_t					*ctp;
	resmgr_iomsgs_t						*msg;
	int									n;

	// Detach thread so a join is not needed
	pthread_detach(pthread_self());

	n = offsetof(resmgr_context_t, iov) + ctrl->nparts_max * sizeof(ctp->iov[0]);
	if(!(ctp = malloc(n + ctrl->msg_max_size))) {
		pthread_mutex_lock(&ctrl->mutex);
		atomic_sub(&ctrl->waiting, 1);
		atomic_sub(&ctrl->created, 1);
		pthread_mutex_unlock(&ctrl->mutex); 
		pthread_exit(0);
	}
	msg = ctp->msg = (resmgr_iomsgs_t *)((char *)ctp + n);
	ctp->ctrl = ctrl;
	// Zero the 'reserved' field because it's at the same location as
	// 'msg_max_size' in the new style resmgr_context_t and will allow
	// the _resmgr_handler function to tell if it's being invoked by
	// this code or the new dispatch code.
	ctp->reserved = 0;

	// try to make the number of threads waiting equal to THREAD_WAITING_MAX
	pthread_mutex_lock(&ctrl->mutex);
	if(ctrl->waiting < ctrl->increment && ctrl->created < ctrl->maximum) {
		pthread_attr_t				attr;

		if(ctrl->thread_stack_size) {
			(void)pthread_attr_init(&attr);
			(void)pthread_attr_setstacksize(&attr, ctrl->thread_stack_size);
		}
		if(pthread_create(0, ctrl->thread_stack_size ? &attr : 0, _resmgr_thread, ctrl) == EOK) {
			atomic_add(&ctrl->waiting, 1);
			atomic_add(&ctrl->created, 1);
		}
		if(ctrl->thread_stack_size) {
			(void)pthread_attr_destroy(&attr);
		}
	}
	pthread_mutex_unlock(&ctrl->mutex); 

	pthread_cleanup_push(_thread_cleanup, ctp);
	for(;;atomic_add(&ctrl->waiting, 1)) {
		// free some threads if too many are waiting
		pthread_mutex_lock(&ctrl->mutex);
		if(ctrl->waiting > ctrl->hi_water) {
			break;
		}
		pthread_mutex_unlock(&ctrl->mutex); 

		// Block waiting for someone
		ctp->id = -1;
		ctp->rcvid = MsgReceive(ctrl->chid, msg, ctrl->msg_max_size, &ctp->info);

		// If not enough are waiting, start one new one that will start more
		pthread_mutex_lock(&ctrl->mutex);
		if(atomic_sub_value(&ctrl->waiting, 1) < ctrl->lo_water && ctrl->created < ctrl->maximum) {
			pthread_attr_t				attr;

			if(ctrl->thread_stack_size) {
				(void)pthread_attr_init(&attr);
				(void)pthread_attr_setstacksize(&attr, ctrl->thread_stack_size);
			}
			if(pthread_create(0, ctrl->thread_stack_size ? &attr : 0, _resmgr_thread, ctrl) == EOK) {
				atomic_add(&ctrl->waiting, 1);
				atomic_add(&ctrl->created, 1);
			}
			if(ctrl->thread_stack_size) {
				(void)pthread_attr_destroy(&attr);
			}
		}
		pthread_mutex_unlock(&ctrl->mutex); 

		// Doing a network transaction and not all the message was send, so get the rest...
		if(ctp->rcvid > 0 && ctp->info.srcmsglen > ctp->info.msglen && ctp->info.msglen < ctrl->msg_max_size) {
			int						n2;

			if((n2 = MsgRead_r(ctp->rcvid, (char *)msg + ctp->info.msglen,
					ctrl->msg_max_size - ctp->info.msglen, ctp->info.msglen)) < 0) {
				MsgError(ctp->rcvid, -n2);
				continue;
			}
			ctp->info.msglen += n2;
		}

		_resmgr_handler(ctp);
	}
	pthread_cleanup_pop(1);
	pthread_mutex_unlock(&ctrl->mutex);
	pthread_exit(0);
	/* NOTREACHED */
	return( 0 );
}

__SRCVERSION("_resmgr_thread.c $Rev: 155997 $");
