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
#include "resmgr.h"

int resmgr_open_bind(resmgr_context_t *ctp, void *ocb, const resmgr_io_funcs_t *funcs) {
	struct binding					*binding;
	struct link						*link;

	if(!(link = _resmgr_link_query(ctp->id, 1))) {
		errno = ENOENT;
		return -1;
	}

	if(!(binding = calloc(sizeof *binding, 1))) {
		errno = ENOMEM;
		return -1;
	}
	binding->count = 1;
	binding->id = link->id;
	binding->ocb = ocb;
	binding->funcs = funcs ? funcs : link->io_funcs;

	_resmgr_link_return(link, 1);

	if(_resmgr_handle(&ctp->info, binding, _RESMGR_HANDLE_SET) == (void *)-1) {
		free(binding);
		errno = EINVAL;
		return -1;
	}
	return 0;
}

__SRCVERSION("resmgr_open_bind.c $Rev: 153052 $");
