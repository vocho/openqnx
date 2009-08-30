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
// FIX ME #include <sys/mempart.h>
#include "kernel/mempart.h"

int
emm_munmap(PROCESS *prp, uintptr_t addr, size_t len, int flags, part_id_t mpart_id) {
	MemobjDestroyed((void *)_syspage_ptr,
 			CPU_V2P(addr), CPU_V2P(addr) + len - 1, 0, 0);
	_sfree((void *)addr, len);
	return EOK;
}

__SRCVERSION("emm_munmap.c $Rev: 168445 $");
