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

int __parse_oflag(const char *mode) {
	int					oflag;
	const char			*p;

	switch(*mode++) {
	case 'a':
		oflag = O_CREAT | O_WRONLY | O_APPEND;
		break;
	case 'w':
		oflag = O_CREAT | O_WRONLY | O_TRUNC;
		break;
	case 'r':
		oflag = O_RDONLY;
		break;
	default:
		return -1;
	}

	p = mode;
	while(*mode) {
		if(!p || memchr(p, *mode, mode - p) != 0) {
			break;
		}
		switch(*mode++) {
		case '+':
			oflag = (oflag & ~O_ACCMODE) | O_RDWR;
			break;
		case 'x':
			oflag |= O_EXCL;
			break;
		case 'b':
			oflag |= O_BINARY;
			break;
		case 't':
			oflag |= O_TEXT;
			break;
		default:
			p = 0;
			break;
		}
	}
	return oflag;
}

__SRCVERSION("__parse_oflag.c $Rev: 153052 $");
