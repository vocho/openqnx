/* wcsrchr function */
#include <wchar.h>
_STD_BEGIN

_WConst_return wchar_t *(wcsrchr)(const wchar_t *s, wchar_t c)
	{	/* find last occurrence of c in wchar_t s[] */
	const wchar_t *sc;

	for (sc = 0; ; ++s)
		{	/* check another wchar_t */
		if (*s == c)
			sc = s;
		if (*s == L'\0')
			return ((wchar_t *)sc);
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcsrchr.c $Rev: 153052 $");
