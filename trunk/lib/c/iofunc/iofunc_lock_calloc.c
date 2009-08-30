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




//Force re-compile
#include <malloc.h>
#include <sys/iofunc.h>

iofunc_lock_list_t *iofunc_lock_calloc(resmgr_context_t *ctp, iofunc_ocb_t *attr, size_t size) {
	return calloc(size, 1);
}

void iofunc_lock_free(iofunc_lock_list_t *lock, size_t size) {
	free(lock);
}

__SRCVERSION("iofunc_lock_calloc.c $Rev: 153052 $");
