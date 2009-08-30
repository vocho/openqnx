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




#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/neutrino.h>

int dup2(int fd, int fd2) {
	if(fd == -1 || (fd & _NTO_SIDE_CHANNEL) || ConnectServerInfo(0, fd, NULL) != fd) {
		errno = EBADF;
		return -1;
	}
	if(fd2 < 0 || (fd2 & _NTO_SIDE_CHANNEL)) {
		errno = EBADF;
		return -1;
	}
	if(fd == fd2) {
		return fd;
	}
	if(close(fd2) == -1 && errno == EINTR) {
		return -1;
	}
	if((fd = fcntl(fd, F_DUPFD, fd2)) == -1)  {
		if(errno != EINTR) errno = EBADF;
		return -1;
	}
	if(fd != fd2)  {
		while(close(fd) == -1 && errno == EINTR) {
			/* nothing to do */
		}
		errno = EBADF;
		return -1;
	}
	return fd;
}

__SRCVERSION("dup2.c $Rev: 153052 $");
