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

extern MEMMGR	memmgr_rte;

void
init_memmgr(void) {

	if(__cpu_flags & CPU_FLAG_MMU) {
		kprintf("Virtual memory management not supported by stand-alone kernel.\n");
		crash();
	}
	memmgr = memmgr_rte;
 	memmgr.pagesize = privateptr->pagesize;
}

__SRCVERSION("init_nto.c $Rev: 153052 $");
