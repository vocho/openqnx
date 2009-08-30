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
#define _FILE_OFFSET_BITS   32
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

int lockf64(int fd, int function, off64_t size)
{
flock64_t	lockl;
int			ret, cmd;

	memset(&lockl, 0, sizeof(lockl));
	lockl.l_start = 0, lockl.l_whence = SEEK_CUR;
	lockl.l_len = size;
	switch (function) {
	case F_ULOCK:
		cmd = F_SETLKW64, lockl.l_type = F_UNLCK;
		break;
	case F_LOCK:
		cmd = F_SETLKW64, lockl.l_type = F_WRLCK;
		break;
	case F_TLOCK:
		cmd = F_SETLK64, lockl.l_type = F_WRLCK;
		break;
	case F_TEST:
		cmd = F_GETLK64, lockl.l_type = F_WRLCK;
		break;
	default:
		errno = EINVAL;
		return(-1);
	}
	ret = fcntl(fd, cmd, &lockl);
	if (ret != -1 && function == F_TEST && lockl.l_type != F_UNLCK) {
		errno = EACCES;
		return(-1);
	}
	return(ret);
}

__SRCVERSION("lockf64.c $Rev: 153052 $");
