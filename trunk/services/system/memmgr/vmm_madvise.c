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

int 
vmm_madvise(PROCESS *prp, uintptr_t vaddr, size_t len, int flags) {
	int						r;
	struct map_set			ms;
	struct mm_map			*mm;

	r = map_isolate(&ms, &prp->memory->map, vaddr, len, MI_SPLIT);
	if(r != EOK) return r;

	for(mm = ms.first; mm != ms.last->next; mm = mm->next) {
		mm->extra_flags = (mm->extra_flags & ~EXTRA_FLAG_MADV_MASK) | flags;
	}
	map_coalese(&ms);
	return EOK;
}

__SRCVERSION("vmm_madvise.c $Rev: 153052 $");
