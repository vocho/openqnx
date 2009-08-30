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

int
pmm_munmap(PROCESS *prp, uintptr_t addr, size_t len, int flags, part_id_t mpart_id) {
	struct mem_phys_entry				*mem, **pmem;

	for(pmem = (struct mem_phys_entry **)&prp->memory; (mem = *pmem); pmem = &mem->next) {
		if(addr == (uintptr_t)(mem + 1) && len == mem->size) {
			*pmem = mem->next;
			MemobjDestroyed((void *)_syspage_ptr,
				(unsigned)CPU_V2P(addr),
				(unsigned)CPU_V2P((uintptr_t)addr + len - 1), 0, 0);
			mem_free_size += (sizeof *mem + mem->size + 3) & ~3;
			_sfree(mem, sizeof *mem + len);
			return EOK;
		}
	}
	return EINVAL;
}

__SRCVERSION("pmm_munmap.c $Rev: 168445 $");
