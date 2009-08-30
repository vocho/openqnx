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




#include <inttypes.h>
#include <sys/mman.h>
#include <sys/memmsg.h>

int msync(void *addr, size_t len, int flags) {
	mem_ctrl_t						msg;

	msg.i.type = _MEM_CTRL;
	msg.i.subtype = _MEM_CTRL_SYNC;
	msg.i.addr = (uintptr_t)addr;
	msg.i.len = len;
	msg.i.flags = flags;
	return MsgSendnc(MEMMGR_COID, &msg.i, sizeof msg.i, 0, 0);
}

__SRCVERSION("msync.c $Rev: 153052 $");
