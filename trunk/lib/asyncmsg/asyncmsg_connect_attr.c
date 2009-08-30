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

int asyncmsg_connect_attr(int coid, struct _asyncmsg_connection_attr *old_attr, const struct _asyncmsg_connection_attr *new_attr)
{
	struct _asyncmsg_connect_context *acc;
	struct _asyncmsg_connection_descriptor *acd;
	
	if ((acc = _asyncmsg_handle(coid, _ASYNCMSG_HANDLE_LOOKUP, 0)) == NULL)
		return -1;
	acd = &acc->acd;

	_mutex_lock(&acd->mu);
	if (old_attr) {
		*old_attr = acd->attr;
	}
	
	if (new_attr) {
		acd->attr = *new_attr;
		/* I can't change max_buffer_size/buff_size, these
		 * will causing acd->start changed, which means, the
		 * memory kernel is referencing will gone :-(
		 */
	}
	_mutex_unlock(&acd->mu);
	return 0;
}

__SRCVERSION("asyncmsg_connect_attr.c $Rev: 153052 $");
