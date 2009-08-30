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
#include <stdlib.h>
#include <stdarg.h>
#include <alloca.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <share.h>
#include <sys/iofunc.h>

int _connect_object(const char *name, const char *prefix, mode_t mode, int oflag, unsigned file_type,
		unsigned extra_type, unsigned extra_len, const void *extra) {
	char						*path;

	if(*name != '/' || strchr(name + 1, '/')) {
		path = (char *)name;
	} else {
		if(!(path = (char *)alloca(strlen(prefix) + strlen(name) + 1))) {
			errno = ENOMEM;
			return -1;
		}
		strcat(strcpy(path, prefix), name);
	}

	if(oflag & O_CREAT) {
		mode &= ~S_IFMT;
	} else {
		mode = 0;
		extra = 0;
	}
	return _connect(0, path, mode, oflag | O_NOCTTY, SH_DENYNO, _IO_CONNECT_OPEN, 0, _IO_FLAG_RD | _IO_FLAG_WR,
			file_type, extra ? extra_type : 0, extra ? extra_len : 0, extra, 0, 0, 0);
}

__SRCVERSION("_connect_object.c $Rev: 153052 $");
