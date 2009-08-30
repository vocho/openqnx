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
#include <sys/stat.h>
#include <sys/iomsg.h>

int fstat64(int fd, struct stat64 *statp) {
	io_stat_t					msg;

	msg.i.type = _IO_STAT;
	msg.i.combine_len = sizeof msg.i;
	msg.i.zero = 0;
	return MsgSendnc(fd, &msg.i, sizeof msg.i, statp, sizeof *statp) == -1 ? -1 : 0;
}

__SRCVERSION("fstat64.c $Rev: 153052 $");
