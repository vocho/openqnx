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




#include <sys/resmgr.h>
#include "resmgr.h"

const resmgr_io_funcs_t *_resmgr_iofuncs(resmgr_context_t *ctp, struct _msg_info *info) {
	struct binding				*binding;

	if((binding = (struct binding *)_resmgr_handle(info, 0, _RESMGR_HANDLE_FIND)) == (void *)-1) {
		return 0;
	}
	return binding->funcs;
}

__SRCVERSION("_resmgr_iofuncs.c $Rev: 153052 $");
