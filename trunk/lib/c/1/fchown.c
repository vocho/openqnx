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

int fchown(int fd, uid_t uid, gid_t gid) {
	io_chown_t					msg;

	msg.i.type = _IO_CHOWN;
	msg.i.combine_len = sizeof msg.i;
	msg.i.uid = uid;
	msg.i.gid = gid;

	return MsgSendnc(fd, &msg.i, sizeof msg.i, 0, 0);
}

__SRCVERSION("fchown.c $Rev: 153052 $");
