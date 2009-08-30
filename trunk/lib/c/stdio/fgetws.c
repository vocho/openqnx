/* fgetws function */
#include "xwstdio.h"
_STD_BEGIN

wchar_t *(fgetws)(wchar_t *_Restrict buf, int n, FILE *_Restrict str)
	{	/* get a wchar_t line from wide stream */
	wchar_t *s = buf;

	if (n <= 1)
		return (0);
	_Lockfileatomic(str);
	while (0 < --n)
		{	/* get a wide character */
		wint_t wc = fgetwc(str);

		if (wc == WEOF)
			break;
		*s++ = wc;
		if (wc == L'\n')
			break;
		}
	if (s == buf)
		buf = 0;
	else
		*s = L'\0';
	_Unlockfileatomic(str);
	return (buf);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fgetws.c $Rev: 153052 $");
