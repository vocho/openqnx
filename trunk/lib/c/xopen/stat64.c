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




#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS	32
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <sys/iomsg.h>

static int _stat64(const char *path, struct stat64 *statp, int mode) {
	struct _io_stat				s;

	s.type = _IO_STAT;
	s.combine_len = sizeof s;
	s.zero = 0;

	if(_connect_combine(path, mode, O_NONBLOCK | O_LARGEFILE | O_NOCTTY, SH_DENYNO, 0, 0, sizeof s, &s, sizeof *statp, statp) == -1) {
		return -1;
	}
	return 0;
}

int lstat64(const char *path, struct stat64 *statp) {
	return _stat64(path, statp, S_IFLNK);
}

int stat64(const char *path, struct stat64 *statp) {
	return _stat64(path, statp, 0);
}

__SRCVERSION("stat64.c $Rev: 153052 $");
