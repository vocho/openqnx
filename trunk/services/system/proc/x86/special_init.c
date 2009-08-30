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

#include <kernel/nto.h>
#include <sys/fault.h>
#include "externs.h"
#include <sys/resmgr.h>
#include <x86/cpumsg.h>

/* v86.c */
int v86_handle(resmgr_context_t *hdr, x86_cpu_v86_t *msg);

static int
special_handler(message_context_t *mcp, int code, unsigned flags, void *handle) {
	x86_cpu_v86_t				*msg = (void *)mcp->msg;
	resmgr_context_t			*ctp = (resmgr_context_t *)mcp;
	
	switch(msg->i.type) {
	case _X86_CPU_V86:
		return proc_status(ctp, v86_handle(ctp, msg));
	default:
		break;
	}
	return proc_status(ctp, ENOSYS);
}

void
special_init(void) {
	message_attach(dpp, NULL, _CPUMSG_BASE, _CPUMSG_MAX, special_handler, NULL);
}

__SRCVERSION("special_init.c $Rev: 160662 $");
