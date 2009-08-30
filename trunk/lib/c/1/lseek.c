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

off64_t lseek64(int fd, off64_t offset, int whence) {
	io_lseek_t							msg;
	off64_t								off;

	msg.i.type = _IO_LSEEK;
	msg.i.combine_len = sizeof msg.i;
	msg.i.offset = offset;
	msg.i.whence = whence;
	msg.i.zero = 0;
	if(MsgSend(fd, &msg.i, sizeof msg.i, &off, sizeof off) == -1) {
		return -1;
	}
	return off;
}

off64_t tell64(int fd) {
	return lseek64(fd, 0, SEEK_CUR);
}

off_t lseek(int fd, off_t offset, int whence) {
	return lseek64(fd, offset, whence);
}

off_t tell(int fd) {
	return lseek64(fd, 0, SEEK_CUR);
}

__SRCVERSION("lseek.c $Rev: 153052 $");
