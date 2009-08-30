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
#include <unistd.h>

int truncate(const char *path, off_t length)
{
int		fd, rc, saved;

	rc = -1;
	if ((fd = open(path, O_WRONLY | O_NONBLOCK)) != -1) {
		rc = ftruncate(fd, length);
		saved = errno;
		close(fd);
		errno = saved;
	}
	return(rc);
}

__SRCVERSION("truncate.c $Rev: 153052 $");
