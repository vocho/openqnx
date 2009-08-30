/* wcschr function */
#include <wchar.h>
_STD_BEGIN

_WConst_return wchar_t *(wcschr)(const wchar_t *s, wchar_t c)
	{	/* find first occurrence of c in wchar_t s[] */
	for (; *s != c; ++s)
		if (*s == L'\0')
			return (0);
	return ((wchar_t *)s);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcschr.c $Rev: 153052 $");
