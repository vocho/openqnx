/* wmemchr function */
#include <wchar.h>
_STD_BEGIN

 #ifndef _WConst_return
  #define _WConst_return
 #endif	/* _WConst_return */

_WConst_return wchar_t *(wmemchr)(const wchar_t *s,
	wchar_t c, size_t n)
	{	/* find first occurrence of c in wchar_t s[n] */
	for (; 0 < n; ++s, --n)
		if (*s == c)
			return ((wchar_t *)s);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wmemchr.c $Rev: 153052 $");
