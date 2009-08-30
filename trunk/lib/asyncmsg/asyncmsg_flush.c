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




#include "asyncmsg_priv.h"

int asyncmsg_flush(int coid, int mode)
{
	struct _asyncmsg_connect_context *acc;
	struct _asyncmsg_connection_descriptor *acd;
	int ret;
	
	ret = MsgSendAsync(coid);
	if (ret == -1 || mode == ASYNCMSG_FLUSH_NONBLOCK)
	  return ret;
	
	if ((acc = _asyncmsg_handle(coid, _ASYNCMSG_HANDLE_LOOKUP, 0)) == NULL)
	  return -1;

	acd = &acc->acd;
	_mutex_lock(&acd->mu);
	while (acd->num_curmsg) {
		pthread_cond_wait(&acd->block_con, &acd->mu);
	}
	_mutex_unlock(&acd->mu);
	
	return 0;
}

__SRCVERSION("asyncmsg_flush.c $Rev: 153052 $");
