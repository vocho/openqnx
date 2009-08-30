/* mbstowcs function */
#include <limits.h>
#include <stdlib.h>
#include "xwchar.h"
_STD_BEGIN

size_t (mbstowcs)(wchar_t *_Restrict wcs, const char *_Restrict s,
	size_t n)
	{	/* translate multibyte string to wide char string */
	int i;
	size_t nw;
	wchar_t wc;
	_Mbstinit(mbst);

	for (nw = 0; nw < n || s == 0; ++nw)
		{	/* make another wide character */
		i = _Mbtowc(&wc, s, INT_MAX, &mbst);
		if (i < 0)
			return ((size_t)-1);
		if (wcs != 0)
			wcs[nw] = wc;
		if (wc == L'\0')
			break;
		s += i;
		}
	return (nw);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mbstowcs.c $Rev: 153052 $");
