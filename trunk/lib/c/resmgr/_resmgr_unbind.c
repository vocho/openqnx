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
#include <sys/resmgr.h>
#include <atomic.h>
#include "resmgr.h"

int _resmgr_unbind(struct _msg_info *rep) {
	struct binding					*binding;

	if((binding =_resmgr_handle(rep, NULL, _RESMGR_HANDLE_REMOVE_LOCK)) == (void *)-1)
		return -1;

	if(atomic_sub_value(&binding->count, 1) == 1)
		free(binding);


	return 0;
}

__SRCVERSION("_resmgr_unbind.c $Rev: 153052 $");
