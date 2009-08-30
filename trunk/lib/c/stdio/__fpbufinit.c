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
#include "xstdio.h"
_STD_BEGIN

void __fpbufinit(FILE *fp) {
	int			save_errno = errno;

	fp->_Buf = &fp->_Cbuf;
	fp->_Next = &fp->_Cbuf;
	fp->_Rend = &fp->_Cbuf;
	fp->_WRend = &fp->_Cbuf;
	fp->_Wend = &fp->_Cbuf;
	fp->_WWend = &fp->_Cbuf;
	fp->_Rback = fp->_Back + sizeof fp->_Back / sizeof *fp->_Back;
	fp->_WRback = fp->_WBack + sizeof fp->_WBack / sizeof *fp->_WBack;
	fp->_Rsave = 0;

	if(isatty(fileno(fp))) {
		fp->_Mode |= _MISTTY;
		setvbuf(fp, 0, _IOLBF, BUFSIZ);
	}
	errno = save_errno;
}

_STD_END

__SRCVERSION("__fpbufinit.c $Rev: 153052 $");
