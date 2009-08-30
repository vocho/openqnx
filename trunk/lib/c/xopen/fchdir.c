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
#include <limits.h>
#include <sys/iomgr.h>
#include <unistd.h>

int fchdir(int fd)
{
int		rwmode;
char	buffer[PATH_MAX + 1];

	if ((rwmode = fcntl(fd, F_GETFL)) != -1) {
		if ((rwmode &= O_ACCMODE) == O_RDWR || rwmode == O_WRONLY)
			errno = ENOTDIR;
		else if (iofdinfo(fd, 0, NULL, buffer, sizeof(buffer)) != -1)
			return(chdir(buffer));
	}
	return(-1);
}

__SRCVERSION("fchdir.c $Rev: 153052 $");
