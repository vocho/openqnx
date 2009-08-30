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




#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <sys/iomsg.h>
#include <unistd.h>

int readblock(int fd, size_t blksize, unsigned block, int numblks, void *buff) {
	struct seek_read {
		struct _io_lseek		lseek;
		struct _io_read			read;
	}						msg;
	ssize_t					nbytes;

	if(numblks > INT_MAX / blksize || numblks < 0) {
		errno = EINVAL;
		return -1;
	}

	msg.lseek.type = _IO_LSEEK;
	msg.lseek.combine_len = offsetof(struct seek_read, read) | _IO_COMBINE_FLAG;
	msg.lseek.offset = (uint64_t)blksize * block;
	msg.lseek.whence = SEEK_SET;
	msg.lseek.zero = 0;
	msg.read.type = _IO_READ;
	msg.read.combine_len = sizeof msg.read;
	msg.read.nbytes = blksize * numblks;
	msg.read.xtype = _IO_XTYPE_NONE;
	msg.read.zero = 0;

	if((nbytes = MsgSend(fd, &msg, offsetof(struct seek_read, read) + sizeof msg.read, buff, msg.read.nbytes)) == -1) {
		return -1;
	}

	return nbytes / (int)blksize;
}

__SRCVERSION("readblock.c $Rev: 153052 $");
