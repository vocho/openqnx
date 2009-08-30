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

int truncate64(const char *path, off64_t length)
{
int		fd, rc, saved;

	rc = -1;
	if ((fd = open64(path, O_WRONLY | O_NONBLOCK)) != -1) {
		rc = ftruncate64(fd, length);
		saved = errno;
		close(fd);
		errno = saved;
	}
	return(rc);
}

__SRCVERSION("truncate64.c $Rev: 153052 $");
