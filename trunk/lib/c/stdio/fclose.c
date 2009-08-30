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
#include "xmtx.h"

_STD_BEGIN

int fclose(FILE *fp) {
	FILE						*p, **pp;
	int							ret;

	if(!fp) {
		return -1;
	}

	_Lockfileatomic(fp);
	_Locksyslock(_LOCK_STREAM);
	for(p = 0, pp = &_Files[0]; (p = *pp); pp = &p->_NextFile) {
		if(p == fp) {
			break;
		}
	}
	if(!p || (fp->_Mode & (_MOPENR | _MOPENW)) == 0) {
		errno = EINVAL;
		_Unlocksyslock(_LOCK_STREAM);
		_Unlockfileatomic(fp);
		return -1;
	}
	if(fp->_Mode & _MALFIL) {
		*pp = fp->_NextFile;
		for(pp = _Files; pp < &_Files[sizeof _Files / sizeof *_Files]; pp++) {
			if(*pp == p) {
				*pp = 0;
				break;
			}
		}
	}
	_Unlocksyslock(_LOCK_STREAM);

	ret = fflush(fp);
	if(close(fileno(fp)) == -1) {
		ret = EOF;
	}
	if(fp->_Mode & _MALBUF) {
		free(fp->_Buf);
	}
	fp->_Buf = 0;
	
	if(fp->_Mode & _MALFIL) {
		_Unlockfileatomic(fp);
		_Releasefilelock(fp);
		free(fp);
	} else {
		fp->_Mode = 0;
		fp->_Handle = -1;
		fp->_Buf = &fp->_Cbuf;
		fp->_Next = &fp->_Cbuf;
		fp->_Rend = &fp->_Cbuf;
		fp->_WRend = &fp->_Cbuf;
		fp->_Wend = &fp->_Cbuf;
		fp->_WWend = &fp->_Cbuf;
		fp->_Rback = fp->_Back + sizeof(fp->_Back);
		fp->_WRback = fp->_WBack + sizeof(fp->_WBack) / sizeof(wchar_t);
		_Unlockfileatomic(fp);
	}		
	return ret;
}

_STD_END

__SRCVERSION("fclose.c $Rev: 153052 $");
