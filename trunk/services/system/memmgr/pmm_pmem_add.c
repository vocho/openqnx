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

#include "pmm.h"

uintptr_t		user_addr = ~(uintptr_t)0;
uintptr_t		user_addr_end = 0;


#define MEM_ALIGNMENT	(sizeof(uint64_t))
int
pmm_pmem_add(paddr_t start, paddr_t len) {
	uintptr_t	addr;
	uintptr_t	end;
	uintptr_t	size;

	// adjust to make sure addr and size fits within a page
	addr = ((uintptr_t)start + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT - 1);
	if(addr < user_addr) user_addr = addr;
	end = ((uintptr_t)(start + len) & ~(MEM_ALIGNMENT - 1)) - 1;
	if(end > user_addr_end) user_addr_end = end;
	if(end > addr) {
		size = (end - addr) + 1;
		mem_free_size += size;
		_sfree((void *)CPU_P2V(addr), size);
	}
	return EOK;
}


__SRCVERSION("pmm_pmem_add.c $Rev: 153052 $");
