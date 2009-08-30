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




#include <string.h>
#include <fcntl.h>
#include <share.h>
#include <sys/dcmd_blk.h>
#include <sys/statvfs.h>
#include <sys/iomsg.h>

int statvfs(const char *path, struct statvfs *statvfsp) {
	extern int					__statvfs_check(const struct statvfs *);
	struct _io_devctl			i;
	struct {
		struct _io_devctl_reply		r;
		struct statvfs				s;
	}							o;

	// Stuff the message.
	i.type = _IO_DEVCTL;
	i.combine_len = sizeof i;
	i.dcmd = DCMD_FSYS_STATVFS;
	i.nbytes = sizeof o.s;
	i.zero = 0;

	if(_connect_combine(path, 0, O_NONBLOCK | O_NOCTTY, SH_DENYNO, 0, 0, sizeof i, &i, sizeof o, &o) == -1) {
		return -1;
	}
	memcpy(statvfsp, &o.s, sizeof *statvfsp);
	return(__statvfs_check(statvfsp));
}

__SRCVERSION("statvfs.c $Rev: 153052 $");
