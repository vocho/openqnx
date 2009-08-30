/* scanf function */
#include "xstdio.h"
_STD_BEGIN

static int scin(void *str, int ch, int getfl)
	{	/* get or put a character */
	return (getfl ? fgetc((FILE *)str)
		: ungetc(ch, (FILE *)str));
	}

int (scanf)(const char *_Restrict fmt, ...)
	{	/* read formatted from stdin */
	int ans;
	va_list ap;

	va_start(ap, fmt);
	_Lockfileatomic(stdin);
	ans = _Scanf(&scin, stdin, fmt, ap, 0);
	_Unlockfileatomic(stdin);
	va_end(ap);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("scanf.c $Rev: 153052 $");
