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
#include <string.h>
#include <sys/iomsg.h>

int ionotify(int fd, int action, int flags, const struct sigevent *event) {
	io_notify_t				msg;

	msg.i.type = _IO_NOTIFY;
	msg.i.combine_len = sizeof msg.i;
	msg.i.action = action;
	msg.i.flags = flags;
	if(event) {
		msg.i.event = *event;
	} else {
		memset(&msg.i.event, 0x00, sizeof msg.i.event);
	}
	if(MsgSend(fd, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o) == -1) {
		return -1;
	}
	return msg.o.flags;
}

__SRCVERSION("ionotify.c $Rev: 153052 $");
