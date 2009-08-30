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




#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <share.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>

_STD_BEGIN

FILE *__fsopen(const char *filename, const char *mode, int sflag, int large) {
	int								fd;
	int								oflag;
	FILE							*fp;
	int								__parse_oflag(const char *mode);

	if((oflag = __parse_oflag(mode)) == -1) {
		errno = EINVAL;
		return 0;
	}
	if((fd = sopen(filename, oflag | large, sflag, 0666)) == -1) {
		return 0;
	}
	if((fp = fdopen(fd, mode))) {
		return fp;
	}
	close(fd);
	return 0;
}

FILE *_fsopen(const char *filename, const char *mode, int sflag) {
	return(__fsopen(filename, mode, sflag, 0));
}

_STD_END


__SRCVERSION("_fsopen.c $Rev: 153052 $");
