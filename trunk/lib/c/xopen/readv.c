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
#include <sys/uio.h>

ssize_t readv(int fd, const iov_t *iov, int nparts) {
	io_read_t					msg;
	int							n;

	msg.i.type = _IO_READ;
	msg.i.combine_len = sizeof msg.i;
	for(msg.i.nbytes = 0, n = -1; ++n < nparts; msg.i.nbytes += GETIOVLEN(&iov[n])) {
		/* nothing to do */
	}
	msg.i.xtype = _IO_XTYPE_NONE;
	msg.i.zero = 0;
	return MsgSendv(fd, (iov_t *)&msg.i, -sizeof msg.i, iov, nparts);
}

__SRCVERSION("readv.c $Rev: 153052 $");
