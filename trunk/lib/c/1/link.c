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
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <share.h>
#include <string.h>
#include <sys/pathmgr.h>
#include <sys/pathmsg.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include "connect.h"

int flink(int fd, const char *path) {
	io_link_extra_t				extra;

	if(ConnectServerInfo(0, fd, &extra.info) != fd) {
		return -1;
	}
	extra.info.nd = netmgr_remote_nd(extra.info.nd, ND_LOCAL_NODE);
	extra.info.pid = getpid();
	extra.info.coid = fd;

	if((fd = _connect(PATHMGR_COID, path, 0, O_CREAT | O_EXCL | O_NOCTTY, SH_DENYNO, _IO_CONNECT_LINK, 0, 0, 0,
			_IO_CONNECT_EXTRA_LINK, sizeof extra, &extra, 0, 0, 0)) == -1) {
		errno = (errno == ENXIO) ? EXDEV : errno;
		return -1;
	}
	ConnectDetach(fd);
	return 0;
}

int link(const char *old, const char *new) {
	int								ret;
	int								save_errno;
	int								fd;
	
	if((fd = _connect(PATHMGR_COID, old, 0, O_LARGEFILE | O_NOCTTY, SH_DENYNO, _IO_CONNECT_OPEN, 0, 0, 0, 0, 0, 0, 0, 0, 0)) == -1) {
		return -1;
	}
	ret = flink(fd, new);
	save_errno = errno;
	close(fd);
	errno = save_errno;
	return ret;
}

__SRCVERSION("link.c $Rev: 153052 $");
