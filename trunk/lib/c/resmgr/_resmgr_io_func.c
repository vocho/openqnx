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

_resmgr_func_t _resmgr_io_func(const resmgr_io_funcs_t *funcs, unsigned type) {
	if(funcs) {
		if(type >= _IO_READ) {
			type -= _IO_READ;
			if(type < funcs->nfuncs) {
				return *(((_resmgr_func_t *)(&funcs->read)) + type);
			}
		}
	}
	return 0;
}

__SRCVERSION("_resmgr_io_func.c $Rev: 153052 $");
