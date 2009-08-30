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
#include <devctl.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/iomsg.h>
#include <sys/dcmd_blk.h>

int fstatvfs(int fd, struct statvfs *statvfsp) {
extern int		__statvfs_check(const struct statvfs *);

	if (_devctl(fd, DCMD_FSYS_STATVFS, statvfsp, sizeof *statvfsp, _DEVCTL_FLAG_NORETVAL) == -1)
		return(-1);
	return(__statvfs_check(statvfsp));
}

__SRCVERSION("fstatvfs.c $Rev: 153052 $");
