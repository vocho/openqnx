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




#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <devctl.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>

int devctl(int fd, int dcmd, void *data_ptr, size_t nbytes, int *info_ptr) {
	io_devctl_t			msg;
	iov_t				iov[4];
	int					status;

	// Stuff the message.
	msg.i.type = _IO_DEVCTL;
	msg.i.combine_len = sizeof msg.i;
	msg.i.dcmd = dcmd;
	msg.i.nbytes = nbytes;
	msg.i.zero = 0;

	// Setup data to the device.
	SETIOV(iov + 0, &msg.i, sizeof msg.i);
	SETIOV(iov + 1, data_ptr, (dcmd & DEVDIR_TO) ? nbytes : 0);

	// Setup data from the device.
	SETIOV(iov + 2, &msg.o, sizeof msg.o);
	SETIOV(iov + 3, data_ptr, (dcmd & DEVDIR_FROM) ? nbytes : 0);

	if((status = MsgSendv_r(fd, iov + 0, GETIOVLEN(&iov[1]) ? 2 : 1, iov + 2, GETIOVLEN(&iov[3]) ? 2 : 1)) != EOK) {
		return status == -ENOSYS ? ENOTTY : -status;
	}

	if(info_ptr) {
		*info_ptr = msg.o.ret_val;
	}

	return EOK;
}

__SRCVERSION("devctl.c $Rev: 153052 $");
