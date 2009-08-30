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




#undef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	32
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/iomsg.h>

off_t ltrunc(int fd, off_t offset, int whence) {
	io_space_t							msg;

	msg.i.type = _IO_SPACE;
	msg.i.combine_len = sizeof msg.i;
	msg.i.subtype = F_FREESP64;
	msg.i.start = offset;
	msg.i.whence = whence;
	msg.i.len = 0;
	if(MsgSend(fd, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o) == -1) {
		return -1;
	}
	return msg.o;
}

__SRCVERSION("ltrunc.c $Rev: 153052 $");
