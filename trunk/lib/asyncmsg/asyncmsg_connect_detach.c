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
#include <errno.h>
#include <sys/mman.h>
#include "asyncmsg_priv.h"

int asyncmsg_connect_detach(int coid)
{
	struct _asyncmsg_connect_context *acc;
	struct _asyncmsg_connection_descriptor *acd;
	struct _asyncmsg_put_header *aph;
	
	if (ConnectDetach(coid) == -1)
	  return -1;
	
	if ((acc = _asyncmsg_handle(coid, _ASYNCMSG_HANDLE_DELETE, 0)) == NULL)
	  return -1;
	
	timer_delete(acc->acd.ttimer);
	
	/* if there is still message in the queue, remove them. The q is like:
	 * 
	 *   (free slot we know)  --- (kernel point here) --- (lib point here)
	 *         free                     head                   tail
	 * 
	 *   free to head are already processed by kernel,
	 *   head to tail are messages already aysncmsg_put() but not processed,
	 *   tail to free are free slot for further asyncmsg_put().
	 */
	acd = &acc->acd;
	_mutex_lock(&acd->mu);
	while (acd->sendq_free != acd->sendq_tail) {
		aph = &acd->sendq[acd->sendq_free];
		/* cast to int to protect wrap over */
		if ((int)(acd->sendq_free - acd->sendq_head) >= 0) {
			aph->err = EBADF;
		}
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
	pthread_cond_destroy(&acc->acd.block_con);
	pthread_mutex_destroy(&acc->acd.mu);
	munmap(acc, sizeof(*acc));
	
	return 0;
}

__SRCVERSION("asyncmsg_connect_detach.c $Rev: 153052 $");
