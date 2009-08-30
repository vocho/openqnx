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




#include <stdarg.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <share.h>
#include <sys/neutrino.h>
#include <sys/ftype.h>
#include <sys/pathmgr.h>
#include <sys/pathmsg.h>
#include <sys/resmgr.h>

int pathmgr_link(const char *path, uint32_t nd, pid_t pid, int chid, unsigned handle, enum _file_type file_type, unsigned flags) {
	struct _io_resmgr_link_extra	linkl;
	int								fd;

	if(flags & _RESMGR_FLAG_FTYPEALL) {
		// Ignore passed in ftype
		file_type = _FTYPE_ALL;
	} else if(file_type < _FTYPE_ANY) {
		errno = EINVAL;
		return -1;
	}

	if(!path || !*path) {
		path = "/";
		flags |= _RESMGR_FLAG_FTYPEONLY;
	} else if(flags & _RESMGR_FLAG_FTYPEONLY) { 	//You must pass in a null path to use FTYPEONLY
		errno = EINVAL;
		return -1;
	}

	linkl.nd = nd;
	linkl.pid = pid ? pid : getpid();
	linkl.chid = chid;
	linkl.handle = handle;
	linkl.file_type = file_type;
	linkl.flags = flags;
	
	if((fd = _connect(PATHMGR_COID, path, 0, O_NOCTTY, SH_DENYNO, _IO_CONNECT_LINK, 0, 0, _FTYPE_LINK,
			_IO_CONNECT_EXTRA_RESMGR_LINK, sizeof linkl, &linkl, 0, 0, 0)) == -1) {
		return -1;
	}

	if(flags & PATHMGR_FLAG_STICKY) {
		ConnectDetach(fd);
		fd = 0;
	}
	return fd;
}

__SRCVERSION("pathmgr_link.c $Rev: 153052 $");
