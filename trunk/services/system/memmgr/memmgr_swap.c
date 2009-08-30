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

#include "externs.h"
#include "mm_internal.h"


/*
 * Called when we run out of memory; try to swap out all we can.
 */

int 
memmgr_swap_freemem(pid_t pid, unsigned size, unsigned waitl) {
	//FUTURE: stub implementation
	return 0;
}

int 
memmgr_swap(resmgr_context_t *ctp, PROCESS *prp, mem_swap_t *msg) {
	//FUTURE: stub implementation
	return ENOSYS;
}

__SRCVERSION("memmgr_swap.c $Rev: 153052 $");
