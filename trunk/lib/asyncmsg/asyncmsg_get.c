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
#include "asyncmsg_priv.h"

struct _asyncmsg_get_header* asyncmsg_get(int chid)
{
	struct _asyncmsg_get_header *agh;
	struct _asyncmsg_channel_context *acc;
	iov_t  *iov;
	size_t entsize;
	int n, used;
	
	if ((acc = _asyncmsg_handle(chid, _ASYNCMSG_HANDLE_LOOKUP | _ASYNCMSG_HANDLE_CHANNEL, 0)) == NULL)
	  return NULL;

	entsize = sizeof(struct _asyncmsg_get_header) + sizeof(iov_t) + acc->buffer_size;
	
	_mutex_lock(&acc->mutex);
	while (acc->num_free < acc->max_num_buffer) {
		void *bufs[1];

		if (!acc->recvbuf_cb) {
			if ((agh = asyncmsg_malloc(entsize)) == NULL) {
				break;
			}
			memset(agh, 0, entsize);
			agh->iov = (iov_t *)(agh + 1);
			agh->iov->iov_base = agh->iov + 1;
			agh->iov->iov_len  = acc->buffer_size;
			agh->parts = 1;
			agh->next = acc->free;
			acc->free = agh;
			acc->num_free++;
		} else {
			if (acc->recvbuf_cb(entsize, 1, (void **)bufs, ASYNCMSG_RECVBUF_ALLOC) <= 0) {
				break;
			}
			agh = bufs[0];
			memset(agh, 0, entsize);
			agh->iov = (iov_t *)(agh + 1);
			agh->iov->iov_base = agh->iov + 1;
			agh->iov->iov_len  = acc->buffer_size;
			agh->parts = 1;
			agh->next = acc->free;
			acc->free = agh;
			acc->num_free++;
		}
	}

	iov = acc->iovs;
	for (n = 0, agh = acc->free; n < acc->max_num_buffer && agh; n++, agh = agh->next)
	{
		SETIOV(&iov[n], agh, entsize);
	}

	if (n == 0) {
		_mutex_unlock(&acc->mutex);
		return NULL;
	}

	if (agh) {
		acc->free = agh;
		agh = iov[n - 1].iov_base;
		agh->next = NULL;
	} else {
		acc->free = NULL;
	}
	acc->num_free -= n;
	_mutex_unlock(&acc->mutex);

	if ((used = MsgReceiveAsync(chid, iov, n)) <= 0) {
		_mutex_lock(&acc->mutex);
		agh = iov[n - 1].iov_base;
		agh->next = acc->free;
		acc->free = iov[0].iov_base;
		acc->num_free += n;
		_mutex_unlock(&acc->mutex);
		return NULL;
	}
	
	if (used < n) {
		_mutex_lock(&acc->mutex);
		agh = iov[n - 1].iov_base;
		agh->next = acc->free;
		acc->free = iov[used].iov_base;

		agh = iov[used - 1].iov_base;
		agh->next = NULL;

		acc->num_free += n - used;
		_mutex_unlock(&acc->mutex);
	}
	agh = iov[0].iov_base;
	return agh;
}

__SRCVERSION("asyncmsg_get.c $Rev: 157149 $");
