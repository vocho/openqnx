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
#include <alloca.h>
#include <sys/iomsg.h>
#include <sys/uio.h>

ssize_t writev(int fd, const iov_t *iov, int nparts) {
	iov_t						*newiov, *p; 
	int							i;
	io_write_t					msg;

	++nparts;
	if(!(p = newiov = alloca(nparts * sizeof *newiov))) {
		errno = ENOMEM;
		return -1;
	}
	msg.i.type = _IO_WRITE;
	msg.i.combine_len = sizeof msg.i;
	msg.i.xtype = _IO_XTYPE_NONE;
	msg.i.nbytes = 0;
	msg.i.zero = 0;
	for(i = 1; i < nparts; i++) {
		msg.i.nbytes += GETIOVLEN(iov);
		*++p = *iov++;
	}
	SETIOV(newiov + 0, &msg.i, sizeof msg.i);
	return MsgSendv(fd, newiov, nparts, 0, 0);
}

__SRCVERSION("writev.c $Rev: 153052 $");
