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




#include <stddef.h>
#include <unistd.h>
#include <sys/memmsg.h>


static int 
_mem_get_info(int fd, int flags, struct posix_typed_mem_info *info) {
	mem_info_t						msg;
	iov_t							iov[3];

	msg.i.type = _MEM_INFO;
	msg.i.zero = 0;
	msg.i.fd = fd;
	msg.i.flags = flags;
	SETIOV(iov + 0, &msg.i, sizeof msg.i);
	SETIOV(iov + 1, &msg.o, offsetof(struct _mem_info_reply, info));
	SETIOV(iov + 2, info, sizeof *info);
	
	return MsgSendvnc(MEMMGR_COID, iov + 0, 1, iov + 1, 2);
}


int 
posix_typed_mem_get_info(int fd, struct posix_typed_mem_info *info) {
	return _mem_get_info(fd, 0, info);
}


/* This was 1003.1j D5 and posix_types_mem_get_info should be used instead */

int (mem_get_info)(int fd, int flags, struct mem_info *info) {
	return _mem_get_info(fd, flags, (struct posix_typed_mem_info *)info);
}

__SRCVERSION("mem_get_info.c $Rev: 153052 $");
