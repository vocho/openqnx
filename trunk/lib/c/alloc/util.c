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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

//Must use <> include for building libmalloc.so
#include <malloc-lib.h>

void 
_malloc_log(char *fmt, ...)
{
	char     buf[1024];
	va_list  v;
	int save_errno = errno;
	extern int _malloc_check_fd;

	if (!_malloc_check_fd || !environ)
		return;

	va_start(v, fmt);
	(void)vsprintf(buf, fmt, v);
	write(_malloc_check_fd, buf, strlen(buf));
	errno = save_errno;
}

static char *mcheck_errs[] = 
{
	"okay",
	"underrun",
	"overrun",
	"freed block"
};

static char *
mcheck_error(enum mcheck_status status)
{
	return mcheck_errs[(int)status];
}

void
_malloc_error(const char *fn, unsigned lno, const char * const msg)
{
	_malloc_log("%s:%d - fatal alloc error - %s\n", fn, lno, msg);
	*((ulong_t *)-1) = 0;   /* fault; generate a dump */
	_exit(1);
}

#if defined(__WATCOMC__)
	#define get_return_addr()			0
#elif defined(__MIPS__) || defined(__SH__) || defined(__ARM__)
	// These processors don't support "1" as an argument to the built-in.
	#define get_return_addr() 			((int)__builtin_return_address(0))
#else
	#define get_return_addr() 			((int)__builtin_return_address(1))
#endif

void
malloc_abort(enum mcheck_status status)
{
	// call user supplied error function _or_
  // standard function.
	if (_malloc_abort != NULL) {
		(*_malloc_abort)(status);
	}
	else {
		_malloc_error("mprobe", get_return_addr(), mcheck_error(status));
	}
}


__SRCVERSION("util.c $Rev: 153052 $");
