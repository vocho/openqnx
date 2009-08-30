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




#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

int flock(int fd, int operation) {
	flock_t lockl;

	memset(&lockl, 0, sizeof(lockl));
	lockl.l_start = 0;
	lockl.l_whence = SEEK_SET;
	lockl.l_len = 0;

	if (operation & LOCK_SH) {
		lockl.l_type = F_RDLCK;
	} else if (operation & LOCK_EX) {
		lockl.l_type = F_WRLCK;
	} else if (operation & LOCK_UN) {
		lockl.l_type = F_UNLCK;
	} else {
		errno = EINVAL;
		return -1;
	}

	if (operation & LOCK_NB) {
		return fcntl(fd, F_SETLK, &lockl);
	}

	return fcntl(fd, F_SETLKW, &lockl);
}


__SRCVERSION("flock.c $Rev: 153052 $");
