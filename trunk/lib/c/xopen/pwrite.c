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
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/iomsg.h>

ssize_t	pwrite64(int fd, const void *buff, size_t nbytes, off64_t offset) {
	struct write_offset {
		struct _io_write		write;
		struct _xtype_offset	offset;
	}						msg;
	iov_t					iov[3];

	msg.write.type = _IO_WRITE;
	msg.write.combine_len = offsetof(struct write_offset, offset) + sizeof msg.offset;
	msg.write.nbytes = nbytes;
	msg.write.xtype = _IO_XTYPE_OFFSET;
	msg.write.zero = 0;
	msg.offset.offset = offset;

	SETIOV(iov + 0, &msg, offsetof(struct write_offset, offset) + sizeof msg.offset);
	SETIOV(iov + 1, buff, nbytes);

	return MsgSendv(fd, iov + 0, 2, 0, 0);
}

ssize_t	pwrite(int fd, const void *buff, size_t nbytes, off_t offset) {
	return pwrite64(fd, buff, nbytes, offset);
}

__SRCVERSION("pwrite.c $Rev: 153052 $");
