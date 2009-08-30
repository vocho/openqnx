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
#include <sys/mman.h>
#include <unistd.h>

#define MEM_ALIGNMENT	(sizeof(uint64_t))
int
emm_pmem_add(paddr_t start, paddr_t len) {
	uintptr_t	addr;
	uintptr_t	end;

	// adjust to make sure addr and size fits within a page
	addr = ((uintptr_t)start + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT - 1);
	end = ((uintptr_t)(start + len) & ~(MEM_ALIGNMENT - 1));

	if((end > addr) && (end != 0)) {
		_sfree((void *)CPU_P2V(addr), end - addr);
	}
	return EOK;
}

__SRCVERSION("emm_pmem_add.c $Rev: 153052 $");
