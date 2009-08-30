/* wmemset function */
#include <wchar.h>
_STD_BEGIN

wchar_t *(wmemset)(wchar_t *s, wchar_t c, size_t n)
	{	/* store c throughout wchar_t s[n] */
	wchar_t *su = s;

	for (; 0 < n; ++su, --n)
		*su = c;
	return (s);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wmemset.c $Rev: 153052 $");
