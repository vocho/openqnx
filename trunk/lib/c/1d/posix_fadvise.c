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

#include <fcntl.h>
#include <errno.h>
#include <sys/dcmd_all.h>

int posix_fadvise64(int fd, off64_t offset, off64_t len, int advice) {
	int					save_errno;
	struct _fadvise		a;

	a.advice = advice;
	a.offset = offset;
	a.len = len;
	a.spare = 0;

	save_errno = errno;
	if(_devctl(fd, DCMD_ALL_FADVISE, &a, sizeof a, _DEVCTL_FLAG_NORETVAL) != 0) {
		int				err = errno;

		errno = save_errno;
		return err;
	}
	return 0;
}

int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
	return posix_fadvise64(fd, offset, len, advice);
}

__SRCVERSION("posix_fadvise.c $Rev: 153052 $");
