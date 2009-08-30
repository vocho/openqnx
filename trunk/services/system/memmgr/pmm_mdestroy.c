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

void
pmm_mdestroy(PROCESS *prp) {
	struct mem_phys_entry			*mem;

	if(!prp || prp->pid == SYSMGR_PID) {
		crash();
	}

	while((mem = (struct mem_phys_entry *)prp->memory)) {
		prp->memory = (ADDRESS *)mem->next;
		MemobjDestroyed((void *)_syspage_ptr,
			CPU_V2P(mem + 1),
			CPU_V2P((mem + 1) + mem->size - 1), 0, 0);
		mem_free_size += (sizeof *mem + mem->size + 3) & ~3;
		_sfree(mem, sizeof *mem + mem->size);
	}
}

__SRCVERSION("pmm_mdestroy.c $Rev: 153052 $");
