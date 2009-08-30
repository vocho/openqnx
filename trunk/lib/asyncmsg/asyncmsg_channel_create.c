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
#include <stdlib.h>
#include "asyncmsg_priv.h"

/* This is painful. We have to prepare receive buf, and associate
 * it with the chid, so we will know where to receive
 */
int asyncmsg_channel_create(unsigned flags,  mode_t mode, size_t buffer_size, unsigned max_num_buffer, const struct sigevent *ev, int (*recvbuf_callback)(size_t bufsize, unsigned num_bufs, void*bufs[], int flags))
{
	struct _asyncmsg_channel_context *acc;
	int chid;
	
	if ((acc = (struct _asyncmsg_channel_context *)malloc(sizeof(*acc))) == NULL) {
		return -1;
	}
	memset(acc, 0, sizeof(*acc));

	if ((errno = pthread_mutex_init(&acc->mutex, 0)) != EOK) {
		free(acc);
		return -1;
	}

	acc->recvbuf_cb = recvbuf_callback;
	acc->max_num_buffer = max_num_buffer;
	acc->buffer_size = buffer_size;

	if ((acc->iovs = malloc(sizeof(iov_t) * max_num_buffer)) == NULL) {
		free(acc);
		return -1;
	}
	
	if ((chid = ChannelCreateExt(flags | _NTO_CHF_ASYNC, mode, buffer_size, max_num_buffer, ev, NULL)) == -1) {
		pthread_mutex_destroy(&acc->mutex);
		free(acc->iovs);
		free(acc);
		return -1;
	}

	if (_asyncmsg_handle(chid, _ASYNCMSG_HANDLE_ADD | _ASYNCMSG_HANDLE_CHANNEL, acc) == NULL) {
		asyncmsg_channel_destroy(chid);
		free(acc->iovs);
		free(acc);
		return -1;
	}
	
	return chid;
}

__SRCVERSION("asyncmsg_channel_create.c $Rev: 153052 $");
