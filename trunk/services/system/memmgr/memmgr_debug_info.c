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

#include <sys/mman.h>
#include <sys/trace.h>
#include "externs.h"

int 
memmgr_debug_info(resmgr_context_t *ctp, PROCESS *prp, mem_debug_info_t *msg) {
	int ret;

	ret = memmgr.debuginfo(prp, &msg->i);

#if defined(VARIANT_instr)
	if(ret == EOK) {
		iov_t iov[4];
		SETIOV(iov, &prp->pid, sizeof(prp->pid)); 
		SETIOV(iov + 1, &msg->i.vaddr, sizeof(msg->i.vaddr)); 
		SETIOV(iov + 2, &msg->i.size, sizeof(msg->i.size)); 
		SETIOV(iov + 3, msg->i.path, strlen(msg->i.path));
		(void)KerextAddTraceEventIOV(_TRACE_SYSTEM_C, _NTO_TRACE_SYS_MAPNAME, iov, 4);
	}
#endif

	return ret;
}

__SRCVERSION("memmgr_debug_info.c $Rev: 153052 $");
