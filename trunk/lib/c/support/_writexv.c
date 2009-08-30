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
#include <errno.h>
#include <sys/iomsg.h>
#include <sys/uio.h>

ssize_t _writexv(int fd, iov_t *iov, int nparts, unsigned xtype, void *xdata, size_t xdatalen, size_t nbytes) {
	io_write_t				msg;

	if(nparts < 1 || (int)xdatalen < 0) {
		errno = EINVAL;
		return -1;
	}
	msg.i.type = _IO_WRITE;
	msg.i.combine_len = sizeof msg.i;
	msg.i.xtype = xtype;
	msg.i.zero = 0;
	SETIOV(iov + 0, &msg.i, sizeof msg.i);
	if((msg.i.nbytes = nbytes) == 0) {
		int						i;

		for(i = 1; i < nparts; i++) {
			msg.i.nbytes += GETIOVLEN(&iov[i]);
		}
	}
	/* If the parts is negative, then the iov points to -nparts bytes of data */
	return MsgSendv(fd, iov, nparts, xdata, -xdatalen);
}

ssize_t _writex(int fd, const void *buff, size_t nbytes, unsigned xtype, void *xdata, size_t xdatalen) {
	iov_t				iov[2];

	SETIOV(iov + 1, buff, nbytes);
	return _writexv(fd, iov, 2, xtype, xdata, xdatalen, nbytes);
}

__SRCVERSION("_writexv.c $Rev: 153052 $");
