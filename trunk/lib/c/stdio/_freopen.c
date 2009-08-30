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
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "file.h"
#include "xstdio.h"

_STD_BEGIN

FILE *_freopen(const char *filename, const char *mode, FILE *fp, int large)
{
int		oflag, fd, err;

	if ((oflag = __parse_oflag(mode)) == -1) {
		fclose(fp);
		errno = EINVAL;
		return(NULL);
	}
	_Lockfileatomic(fp);
	fflush(fp);
	if (filename == NULL) {
		if ((fd = openfd(_FD_NO(fp), oflag | large)) == -1) {
			err = (errno == EACCES) ? EBADF : errno;
			_Unlockfileatomic(fp);
			fclose(fp);
			errno = err;
			return(NULL);
		}

		/*
		 * Attempt to keep the same file descriptor since the POSIX
		 * spec says that freopen(NULL,...) does not need to close
		 * the existing file descriptor.
		 *
		 * The filename case requires the old file descriptor to be
		 * closed and a new file descriptor allocated as the lowest
		 * numbered unused descriptor.
		 */
		if (fd != _FD_NO(fp) && dup2(fd, _FD_NO(fp)) != -1) {
			close(fd);
			fd = _FD_NO(fp);
		}
	}
	else {
		close(_FD_NO(fp));
		if ((fd = open(filename, oflag | large, 0666)) == -1) {
			err = errno;
			_Unlockfileatomic(fp);
			fclose(fp);
			errno = err;
			return(NULL);
		}
	}
	_Locksyslock(_LOCK_STREAM);
	if (fp->_Mode & _MALBUF)
		free(fp->_Buf);
	fp->_Mode &= _MALFIL;
	__fpinit(fp, fd, oflag);
	_Unlockfileatomic(fp);
	_Unlocksyslock(_LOCK_STREAM);
	return(fp);
}

_STD_END

__SRCVERSION("_freopen.c $Rev: 153052 $");
