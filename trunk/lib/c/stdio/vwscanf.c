/* vwscanf function */
#include "xwstdio.h"
_STD_BEGIN

static wint_t scin(void *str, wint_t ch, int getfl)
	{	/* get or put a wide character */
	return ((wint_t)(getfl ? fgetwc((FILE *)str)
		: ungetwc(ch, (FILE *)str)));
	}

int (vwscanf)(const wchar_t *_Restrict fmt, va_list ap)
	{	/* read formatted from wide stdin to arg list */
	int ans;

	_Lockfileatomic(stdin);
	ans = _WScanf(&scin, stdin, fmt, ap, 0);
	_Unlockfileatomic(stdin);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("vwscanf.c $Rev: 153052 $");
