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




#include <stdarg.h>
#include <fcntl.h>

LIBC_WEAK(sopen, __sopen);

int sopen(const char *path, int oflag, int sflag, ...) {
	va_list				ap;
	unsigned			fd;

	va_start(ap, sflag);
	fd = _vopen(path, oflag, sflag, ap);
	va_end(ap);
	return fd;
}

__SRCVERSION("sopen.c $Rev: 167420 $");
