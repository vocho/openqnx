/* vscanf function */
#include "xstdio.h"
_STD_BEGIN

static int scin(void *str, int ch, int getfl)
	{	/* get or put a character */
	return (getfl ? fgetc((FILE *)str)
		: ungetc(ch, (FILE *)str));
	}

int (vscanf)(const char *_Restrict fmt, va_list ap)
	{	/* read formatted from stdin to arg list */
	int ans;

	_Lockfileatomic(stdin);
	ans = _Scanf(&scin, stdin, fmt, ap, 0);
	_Unlockfileatomic(stdin);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("vscanf.c $Rev: 153052 $");
