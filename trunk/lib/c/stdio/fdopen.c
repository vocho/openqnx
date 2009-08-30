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
#include "file.h"
#include "xstdio.h"
_STD_BEGIN

FILE *fdopen(int fd, const char *mode) {
	int								oflag;
	FILE							*fp, **pfp;

	if((oflag = __parse_oflag(mode)) == -1) {
		errno = EINVAL;
		return 0;
	}
	_Locksyslock(_LOCK_STREAM);
	for(fp = 0, pfp = &_Files[0]; (fp = *pfp); pfp = &fp->_NextFile) {
		if(fp->_Mode == 0) {
			break;
		}
	}
	if(!fp) {
		if(!(fp = (FILE *)calloc(1, sizeof *fp))) {
			_Unlocksyslock(_LOCK_STREAM);
			errno = ENOMEM;
			return 0;
		}
		*pfp = fp;
		for(pfp = _Files; pfp < &_Files[sizeof _Files / sizeof *_Files]; pfp++) {
			if(!*pfp) {
				*pfp = fp;
				break;
			}
		}
		fp->_Mode = _MALFIL;
	}
	_Lockfileatomic(fp);
	__fpinit(fp, fd, oflag);
	_Unlocksyslock(_LOCK_STREAM);
	_Unlockfileatomic(fp);
	return fp;
}

_STD_END

__SRCVERSION("fdopen.c $Rev: 153052 $");
