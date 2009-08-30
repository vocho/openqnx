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





#define _FILE_OFFSET_BITS	32
#include <errno.h>
#include <sys/statvfs.h>

int __statvfs_check(const struct statvfs *st)
{
	if (st->f_blocks_hi != 0 || st->f_bfree_hi != 0 || st->f_bavail_hi != 0 || st->f_files_hi != 0 || st->f_ffree_hi != 0 || st->f_favail_hi != 0) {
		errno = EOVERFLOW;
		return(-1);
	}
	return(0);
}

__SRCVERSION("__statvfs_check.c $Rev: 153052 $");
