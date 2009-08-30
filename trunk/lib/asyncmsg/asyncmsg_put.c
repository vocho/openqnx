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
#include "asyncmsg_priv.h"

static int _asyncmsg_put_trigger(int coid, struct _asyncmsg_connect_context *acc)
{
	/* figure out if we need to trigger kernel */
	acc->acd.num_curmsg++;
	if (acc->acd.attr.trigger_num_msg && acc->acd.num_curmsg >= acc->acd.attr.trigger_num_msg)
	{
		return MsgSendAsync(coid);
	}

	/* if this is the first message, trigger & tick the ttimer */
	if (acc->flags & _ASYNCMSG_CONNECT_TIMEROFF) {
		TimerSettime(acc->acd.ttimer, 0, &acc->acd.attr.trigger_time, NULL);
		acc->flags &= ~_ASYNCMSG_CONNECT_TIMEROFF;
	}
	return 0;
}

int asyncmsg_putv(int coid, const iov_t* iov, int parts, unsigned handle, int (*call_back)(int err, void* buf, unsigned handle))
{
	struct _asyncmsg_connect_context *acc;
	struct _asyncmsg_connection_descriptor *acd;
	struct _asyncmsg_put_header *aph;
	unsigned new_tail;
	int err;
	
	if ((acc = _asyncmsg_handle(coid, _ASYNCMSG_HANDLE_LOOKUP, 0)) == NULL)
	  return -1;
	acd = &acc->acd;
	
	_mutex_lock(&acd->mu);
	for (;;) {
		new_tail = acd->sendq_tail + 1;
		if (new_tail >= acd->sendq_size)
		  new_tail = 0;
		
		if (new_tail != acd->sendq_free)
		  break;
		
		/* put list is full */
		if (acd->flags & _NTO_COF_NONBLOCK) {
			_mutex_unlock(&acd->mu);
			errno = EAGAIN;
			return -1;
		}
		pthread_cond_wait(&acd->block_con, &acd->mu);
	}
	
	aph = &acd->sendq[acd->sendq_tail];
	acd->sendq_tail = new_tail;
	
	aph->err = 0;
	aph->iov = (iov_t *)iov;
	aph->parts = parts;
	aph->handle = handle;
	aph->cb = call_back;
	
	err = _asyncmsg_put_trigger(coid, acc);	
	_mutex_unlock(&acd->mu);
	
	return err;
}

int asyncmsg_put(int coid, const void *buff, size_t size, unsigned handle, int (*call_back)(int err, void* buf, unsigned handle))
{
	return asyncmsg_putv(coid, (iov_t *)buff, (int)-size, handle, call_back);
}


__SRCVERSION("asyncmsg_put.c $Rev: 153052 $");
