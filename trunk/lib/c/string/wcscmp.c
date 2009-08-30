/* wcscmp function */
#include <wchar.h>
_STD_BEGIN

int (wcscmp)(const wchar_t *s1, const wchar_t *s2)
	{	/* compare wchar_t s1[], s2[] */
	for (; *s1 == *s2; ++s1, ++s2)
		if (*s1 == L'\0')
			return (0);
	return (*s1 < *s2 ? -1 : +1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcscmp.c $Rev: 153052 $");
