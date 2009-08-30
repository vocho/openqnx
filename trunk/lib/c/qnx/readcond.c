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

int readcond(int fd, void *buff, int nbytes, int min, int timel, int timeout) {
	struct {
		struct _io_read				read;
		struct _xtype_readcond		readcond;
	}						msg;

	// Set read params
	msg.read.type = _IO_READ;
	msg.read.combine_len = sizeof msg;
	msg.read.nbytes = nbytes;
	msg.read.xtype = _IO_XTYPE_READCOND;
	msg.read.zero = 0;

	// Set readcond params
	msg.readcond.min = min;
	msg.readcond.time = timel;
	msg.readcond.timeout = timeout;

	return MsgSend(fd, &msg, sizeof msg, buff, nbytes);	
}

__SRCVERSION("readcond.c $Rev: 153052 $");
