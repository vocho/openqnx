/* wcscpy function */
#include <wchar.h>
_STD_BEGIN

wchar_t *(wcscpy)(wchar_t *_Restrict s1, const wchar_t *_Restrict s2)
	{	/* copy wchar_t s2[] to s1[] */
	wchar_t *s;

	for (s = s1; (*s++ = *s2++) != L'\0'; )
		;
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcscpy.c $Rev: 153052 $");
