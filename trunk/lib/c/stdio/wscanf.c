/* wscanf function */
#include "xwstdio.h"
_STD_BEGIN

static wint_t scin(void *str, wint_t ch, int getfl)
	{	/* get or put a wide character */
	return ((wint_t)(getfl ? fgetwc((FILE *)str)
		: ungetwc(ch, (FILE *)str)));
	}

int (wscanf)(const wchar_t *_Restrict fmt, ...)
	{	/* read formatted from wide stdin */
	int ans;
	va_list ap;

	va_start(ap, fmt);
#ifdef __QNX__
	_Lockfileatomic(stdin);
	ans = _WScanf(&scin, stdin, fmt, ap, 0);
	_Unlockfileatomic(stdin);
#else
	ans = _WScanf(&scin, stdin, fmt, ap, 0);
#endif
	va_end(ap);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wscanf.c $Rev: 153052 $");
