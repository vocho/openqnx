/* wmemcpy function */
#include <wchar.h>
_STD_BEGIN

wchar_t *(wmemcpy)(wchar_t *_Restrict s1, const wchar_t *_Restrict s2,
	size_t n)
	{	/* copy wchar_t s2[n] to s1[n] in any order */
	wchar_t *su1 = s1;

	for (; 0 < n; ++su1, ++s2, --n)
		*su1 = *s2;
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wmemcpy.c $Rev: 153052 $");
