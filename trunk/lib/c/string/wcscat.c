/* wcscat function */
#include <wchar.h>
_STD_BEGIN

wchar_t *(wcscat)(wchar_t *_Restrict s1, const wchar_t *_Restrict s2)
	{	/* copy wchar_t s2[] to end of s1[] */
	wchar_t *s;

	for (s = s1; *s != L'\0'; ++s)
		;			/* find end of s1[] */
	for (; (*s = *s2) != L'\0'; ++s, ++s2)
		;			/* copy s2[] to end */
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcscat.c $Rev: 153052 $");
