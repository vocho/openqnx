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
#include <fcntl.h>
#include <string.h>
#include <share.h>
#include <sys/iomsg.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX	1024 	/* Should use fpathconf(path, _PC_PATH_MAX) */
#endif

int readlink(const char *path, char *buf, size_t bufsize) {
	struct {
		struct _io_connect_link_reply		r;
		char								path[PATH_MAX + 1];
	}								msg;
	int								fd, len;

	if((fd = _connect(_NTO_SIDE_CHANNEL, path, S_IFLNK, O_NOCTTY, SH_DENYNO, _IO_CONNECT_READLINK, 0, 0, 0, 0, 0, 0, sizeof msg, &msg, 0)) == -1) {
		return -1;
	}
	ConnectDetach(fd);

	// If no len returned from server calculate it
	if(msg.r.path_len == 0) {
		msg.r.path_len = strlen(msg.path);
	}

	// Copy the link path
	memcpy(buf, msg.path, len = min(msg.r.path_len, bufsize));

	// Null terminate to be nice
	if(msg.r.path_len < bufsize) {
		buf[msg.r.path_len] = '\0';
	}

	return len;
}

__SRCVERSION("readlink.c $Rev: 153052 $");
