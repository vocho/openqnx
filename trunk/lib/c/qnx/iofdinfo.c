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
#include <sys/iomgr.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>

int iofdinfo(int fd, unsigned flags, struct _fdinfo *info, char *path, int maxlen) {
	io_fdinfo_t				msg;
	iov_t					iov[3];

	msg.i.type = _IO_FDINFO;
	msg.i.combine_len = sizeof msg.i;
	msg.i.flags = flags;
	msg.i.path_len = path ? maxlen : 0;
	msg.i.reserved = 0;

	SETIOV(iov + 0, &msg.o, offsetof(struct _io_fdinfo_reply, info));
	if(info) {
		SETIOV(iov + 1, info, sizeof msg.o.info);
	} else {
		SETIOV(iov + 1, &msg.o.info, sizeof msg.o.info);
	}
	SETIOV(iov + 2, path, msg.i.path_len);

	return MsgSendsvnc(fd, &msg.i, sizeof msg.i, iov, 3);
}

__SRCVERSION("iofdinfo.c $Rev: 153052 $");
