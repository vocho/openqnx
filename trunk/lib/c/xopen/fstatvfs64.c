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
#include <stdlib.h>
#include <devctl.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/iomsg.h>
#include <sys/dcmd_blk.h>

int fstatvfs64(int fd, struct statvfs64 *statvfsp) {
	return _devctl(fd, DCMD_FSYS_STATVFS, statvfsp, sizeof *statvfsp, _DEVCTL_FLAG_NORETVAL);
}

__SRCVERSION("fstatvfs64.c $Rev: 153052 $");
