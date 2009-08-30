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

int pathmgr_symlink(const char *path, const char *symlinkp) { 
	int							fd;
	
	if((fd = _connect(PATHMGR_COID, symlinkp, S_IFLNK | S_IPERMS, O_CREAT | O_EXCL | O_NOCTTY, SH_DENYNO, _IO_CONNECT_LINK, 0, 0, _FTYPE_LINK,
			_IO_CONNECT_EXTRA_PROC_SYMLINK, strlen(path) + 1, path, 0, 0, 0)) == -1) {
		return -1;
	}
	ConnectDetach(fd);
	return 0;
}

__SRCVERSION("pathmgr_symlink.c $Rev: 153052 $");
