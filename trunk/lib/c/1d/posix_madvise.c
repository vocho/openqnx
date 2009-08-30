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


int 
posix_madvise(void *addr, size_t len, int advice) {
	mem_ctrl_t						msg;
	int								r;

	msg.i.type = _MEM_CTRL;
	msg.i.subtype = _MEM_CTRL_ADVISE;
	msg.i.addr = (uintptr_t)addr;
	msg.i.len = len;
	msg.i.flags = advice;
	r = MsgSendnc_r(MEMMGR_COID, &msg.i, sizeof msg.i, 0, 0);
	if(r > 0) r = 0;
	return -r;
}

__SRCVERSION("posix_madvise.c $Rev: 153052 $");
