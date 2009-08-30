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

#include "vmm.h"


MEMMGR	memmgr_virtual = {
	__PAGESIZE,
	0,
	sizeof(ADDRESS),

	#undef MMF
	#define MMF(r,f,p,e)	MMF_DEFN(r,f,p,e)
	MM_FUNCS(vmm)
};

__SRCVERSION("vmm.c $Rev: 153052 $");
