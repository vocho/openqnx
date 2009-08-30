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




#include <unistd.h>
#include <sys/iomsg.h>

ssize_t write(int fd, const void *buff, size_t nbytes) {
	io_write_t					msg;
	iov_t						iov[2];

	msg.i.type = _IO_WRITE;
	msg.i.combine_len = sizeof msg.i;
	msg.i.xtype = _IO_XTYPE_NONE;
	msg.i.nbytes = nbytes;
	msg.i.zero = 0;
	SETIOV(iov + 0, &msg.i, sizeof msg.i);
	SETIOV(iov + 1, buff, nbytes);
	return MsgSendv(fd, iov, 2, 0, 0);
}

__SRCVERSION("write.c $Rev: 153052 $");
