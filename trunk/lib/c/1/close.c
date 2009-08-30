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

int close(int fd) {
	io_close_t					msg;
	int							ret, err;

	msg.i.type = _IO_CLOSE;
	msg.i.combine_len = sizeof msg.i;
	if((ret = MsgSend(fd, &msg.i, sizeof msg.i, 0, 0)) != -1 || errno != EINTR) {
		if((err = ConnectDetach_r(fd)) == EOK) {
			return ret;
		}
		if(ret != -1) {
			errno = err;
		}
	}
	return -1;
}

__SRCVERSION("close.c $Rev: 153052 $");
