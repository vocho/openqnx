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




#undef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	32
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/iomsg.h>

int ftruncate64(int fd, off64_t length) {
	io_space_t					msg;
	off64_t						size;

	/*
	 We do this test here so as to not impose limits
	 in the resource managers themselves.
	*/
	if(length < 0) {
		errno = EINVAL;
		return -1;
	}

	msg.i.type = _IO_SPACE;
	msg.i.combine_len = sizeof msg.i;
	msg.i.subtype = F_FREESP64;
	msg.i.whence = SEEK_SET;
	msg.i.start = length;
	msg.i.len = 0;		// to end of file

	if(MsgSend(fd, &msg.i, sizeof msg.i, &size, sizeof size) == -1) {
		return -1;
	}

	if(size >= length) {
		return 0;
	}

	msg.i.type = _IO_SPACE;
	msg.i.combine_len = sizeof msg.i;
	msg.i.subtype = F_ALLOCSP64;
	msg.i.whence = SEEK_SET;
	msg.i.start = size;
	msg.i.len = length - size;

	if(MsgSend(fd, &msg.i, sizeof msg.i, 0, 0) == -1) {
		return -1;
	}

	return 0;
}

__SRCVERSION("ftruncate64.c $Rev: 153052 $");
