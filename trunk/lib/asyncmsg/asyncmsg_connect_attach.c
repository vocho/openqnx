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
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "asyncmsg_priv.h"

static struct sigevent *_async_ev;
static pthread_t       _async_ev_tid;

static void * _async_event_thread(void *arg)
{
	int chid = (int)arg, coid;
	struct _pulse pulse;
	struct _asyncmsg_connect_context *acc;
	struct _asyncmsg_connection_descriptor *acd;
	
	for (;;) {
		if (MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1)
		  return NULL;

		coid = pulse.value.sival_int;
		if ((acc = _asyncmsg_handle(coid, _ASYNCMSG_HANDLE_LOOKUP, 0)) == NULL)
		  continue;
		acd = &acc->acd;
		
		_mutex_lock(&acd->mu);
		if (pulse.code == 'T') {
			/* triger timer expired */
			if (acc->acd.num_curmsg) {
				MsgSendAsync(coid);
			}
			acc->flags |= _ASYNCMSG_CONNECT_TIMEROFF;
			_mutex_unlock(&acd->mu);
			continue;
		}
		
		if (pulse.code == 'P') 
		{
			/* collect some done message from "free" ptr */
			struct _asyncmsg_put_header *aph;
			
			while (acd->sendq_free != acd->sendq_head) {
				aph = &acd->sendq[acd->sendq_free];
				if (aph->cb) {
					aph->cb(aph->err, aph->iov, aph->handle);
				} else if (acd->attr.call_back) {
					acd->attr.call_back(aph->err, aph->iov, aph->handle);
				}
						   
				if (++acd->sendq_free >= acd->sendq_size)
				  acd->sendq_free = 0;
			}
			
			/* Sigh! have to use broadcast cause the condvar both 
			 * use for queue full, and asyncmsg_flush()
			 */
			pthread_cond_broadcast(&acd->block_con);
			_mutex_unlock(&acd->mu);
		}
	}
}

int asyncmsg_connect_attach(uint32_t nd, pid_t pid, int chid, unsigned index, unsigned flags, const struct _asyncmsg_connection_attr *attr)
{
	struct _asyncmsg_connect_context *acc;
	struct _asyncmsg_connection_descriptor *acd;
	int id, size;
	static pthread_mutex_t _async_init_mutex = PTHREAD_MUTEX_INITIALIZER;

	_mutex_lock(&_async_init_mutex);
	if (!_async_ev) {
		int chid;
		
		if ((_async_ev = malloc(sizeof(*_async_ev))) == NULL) {
			_mutex_unlock(&_async_init_mutex);
			return -1;
		}
			
		if ((chid = ChannelCreate(0)) == -1) 
		{
			free(_async_ev);
			_async_ev = NULL;
			_mutex_unlock(&_async_init_mutex);
			return -1;
		}
		
		if ((_async_ev->sigev_coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0)) == -1)
		{
			ChannelDestroy(chid);
			free(_async_ev);
			_async_ev = NULL;
			_mutex_unlock(&_async_init_mutex);
			return -1;
		}
		
		_async_ev->sigev_notify = SIGEV_PULSE;
		_async_ev->sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
		
		if ((errno = pthread_create(&_async_ev_tid, NULL, _async_event_thread, (void *)chid)) != EOK) 
		{
			ConnectDetach(_async_ev->sigev_coid);
			ChannelDestroy(chid);
			free(_async_ev);
			_async_ev = NULL;
			_mutex_unlock(&_async_init_mutex);
			return -1;
		}
	}
	_mutex_unlock(&_async_init_mutex);

	size = sizeof(*acc) + sizeof(struct _asyncmsg_put_header) * attr->max_num_buffer;
	acc = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_PHYS, NOFD, 0);
	if (acc == (struct _asyncmsg_connect_context *)MAP_FAILED) {
	  return -1;
	}
	memset(acc, 0, sizeof(*acc));
	acd = &acc->acd;

	flags |= _NTO_COF_NOSHARE;
	acd->attr = *attr;
	acd->flags = flags;
	acd->sendq_size = attr->max_num_buffer;
	acd->sendq = (struct _asyncmsg_put_header *)((char *)acc + sizeof(*acc));

	if ((id = ConnectAttachExt(nd, pid, chid, index, flags, acd)) == -1) {
		return -1;
	}
	
	acd->sendq_head = acd->sendq_tail = acd->sendq_free = 0;
	acd->ev = *_async_ev;
	acd->ev.sigev_value.sival_int = id;
	acd->ev.sigev_code = 'T';
	if ((acd->ttimer = TimerCreate(CLOCK_REALTIME, &acd->ev)) == (timer_t)-1) {
		asyncmsg_connect_detach(id);
		return -1;
	}
	acc->flags = _ASYNCMSG_CONNECT_TIMEROFF;
	
	acd->ev.sigev_code = 'P';
	if ((errno = pthread_mutex_init(&acd->mu, 0)) != EOK)
	{
		asyncmsg_connect_detach(id);
		return -1;	
	}
	
	if ((errno = pthread_cond_init(&acd->block_con, 0)) != EOK)
	{
		asyncmsg_connect_detach(id);
		return -1;
	}
	
	if (_asyncmsg_handle(id, _ASYNCMSG_HANDLE_ADD, acc) == NULL) {
		asyncmsg_connect_detach(id);
		return -1;
	}
	
	return id;
}

__SRCVERSION("asyncmsg_connect_attach.c $Rev: 153052 $");
