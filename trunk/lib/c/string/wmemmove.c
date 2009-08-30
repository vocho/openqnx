/* wmemmove function */
#include <wchar.h>
_STD_BEGIN

wchar_t *(wmemmove)(wchar_t *s1, const wchar_t *s2, size_t n)
	{	/* copy wchar_t s2[n] to s1[n] safely */
	wchar_t *su1 = s1;

	if (s2 < su1 && su1 < s2 + n)
		for (su1 += n, s2 += n; 0 < n; --n)
			*--su1 = *--s2;	/*copy backwards */
	else
		for (; 0 < n; --n)
			*su1++ = *s2++;	/* copy forwards */
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wmemmove.c $Rev: 153052 $");
