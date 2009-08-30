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

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <sys/iomsg.h>
#include <unistd.h>
#include "externs.h"

//FUTURE: Consider using a pread64() style message for this rather
//FUTURE: the combined lseek()/read(). Right now we have problems
//FUTURE: with file systems not supporting pread() (e.g. flash),
//FUTURE: and on the QNX4 FS pread() will turn off the read-ahead
//FUTURE: optimization. jgarvey's thinking about enabling read-ahead
//FUTURE: for contiguous pread's and there are PR's in the system
//FUTURE: about the FS's that don't support it, so maybe in a year
//FUTURE: or two we'll be able to switch. bstecher 2005/01/19.

ssize_t 
proc_read(int fd, void *buff, ssize_t nbytes, off64_t off) {
	struct seek_read {
		struct _io_lseek		lseek;
		struct _io_read			read;
	}						msg;
	ssize_t					received;
	ssize_t					got;

	msg.lseek.type = _IO_LSEEK;
	msg.lseek.combine_len = offsetof(struct seek_read, read) | _IO_COMBINE_FLAG;
	msg.lseek.offset = off;
	msg.lseek.whence = SEEK_SET;
	msg.lseek.zero = 0;
	msg.read.type = _IO_READ;
	msg.read.combine_len = sizeof msg.read;
	msg.read.nbytes = nbytes;
	msg.read.xtype = _IO_XTYPE_NONE;
	msg.read.zero = 0;

	received = MsgSend(fd, &msg, offsetof(struct seek_read, read) + sizeof msg.read, buff, msg.read.nbytes);
	if(received == -1) return -1;

	/* deal with short reads */
	for( ;; ) {
		if(received >= nbytes) break;
		msg.read.nbytes = nbytes - received;
		got = MsgSend(fd, &msg.read, sizeof msg.read, (uint8_t *)buff + received, msg.read.nbytes);
		if(got == -1) return -1;
		if(got == 0) break;
		received += got;
	}

	return received;
}

__SRCVERSION("proc_read.c $Rev: 153052 $");
