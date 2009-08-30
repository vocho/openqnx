/* wcsncpy function */
#include <wchar.h>
_STD_BEGIN

wchar_t *(wcsncpy)(wchar_t *_Restrict s1, const wchar_t *_Restrict s2,
	size_t n)
	{	/* copy wchar_t s2[max n] to s1[n] */
	wchar_t *s;

	for (s = s1; 0 < n && *s2 != L'\0'; --n)
		*s++ = *s2++;	/* copy at most n wchar_ts from s2[] */
	for (; 0 < n; --n)
		*s++ = L'\0';
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcsncpy.c $Rev: 153052 $");
