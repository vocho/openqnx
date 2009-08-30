/* wprintf function */
#include "xwstdio.h"
_STD_BEGIN

#ifdef __QNX__
#define prout _Wfprout
#define args _WPargs
#else
static void *prout(void *arg, const wchar_t *buf, size_t n)
	{	/* write to wide file */
	FILE *str = (FILE *)arg;

	for (; 0 < n; --n, ++buf)
		if (fputwc(*buf, str) == WEOF)
			return (0);
	return (str);
	}
#endif

int (wprintf)(const wchar_t *_Restrict fmt, ...)
	{	/* print formatted to wide stdout */
	int ans;
	va_list ap;

	va_start(ap, fmt);
#ifdef __QNX__
	_Lockfileatomic(stdout);
	ans = _WPrintf(&prout, stdout, fmt, ap);
	_Unlockfileatomic(stdout);
#else
	ans = _WPrintf(&prout, stdout, fmt, ap);
#endif
	va_end(ap);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wprintf.c $Rev: 153052 $");
