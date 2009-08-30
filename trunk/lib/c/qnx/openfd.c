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




#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <share.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>

int _sopenfd(int fd, int oflag, int sflag, int xtype) {
	int								fd2;
	io_openfd_t						msg;
	struct _server_info				info;
	
	if(fd == -1 || ConnectServerInfo(0, fd, &info) != fd) {
		errno = EBADF;
		return -1;
	}

	if((fd2 = ConnectAttach(info.nd, info.pid, info.chid, 0, _NTO_COF_CLOEXEC)) == -1) {
		return -1;
	}

	memset(&msg.i, 0x00, sizeof msg.i);
	msg.i.type = _IO_OPENFD;
	msg.i.combine_len = sizeof msg.i;
	msg.i.ioflag = (oflag & ~(O_ACCMODE | O_CLOEXEC)) | ((oflag + 1) & O_ACCMODE);
	msg.i.sflag = sflag;
	msg.i.xtype = xtype;
	msg.i.info.nd = netmgr_remote_nd(info.nd, ND_LOCAL_NODE);
	msg.i.info.pid = getpid();
	msg.i.info.chid = info.chid;
	msg.i.info.scoid = info.scoid;
	msg.i.info.coid = fd;

	if(MsgSend(fd2, &msg.i, sizeof msg.i, 0, 0) == -1) {
		ConnectDetach(fd2);
		return -1;
	}
	
	if(!(oflag & O_CLOEXEC)) {
		ConnectFlags_r(0, fd2, FD_CLOEXEC, 0);
	}

	return fd2;
}

int sopenfd(int fd, int oflag, int sflag) {
	return _sopenfd(fd, oflag, sflag, _IO_OPENFD_NONE);
}

int openfd(int fd, int oflag) {
	return sopenfd(fd, oflag, SH_DENYNO);
}

__SRCVERSION("openfd.c $Rev: 153052 $");
