/* wcsncmp function */
#include <wchar.h>
_STD_BEGIN

int (wcsncmp)(const wchar_t *s1, const wchar_t *s2, size_t n)
	{	/* compare wchar_t s1[max n], s2[max n] */
	for (; 0 < n; ++s1, ++s2, --n)
		if (*s1 != *s2)
			return ((*(wchar_t *)s1 < *(wchar_t *)s2)
				? -1 : +1);
		else if (*s1 == L'\0')
			return (0);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcsncmp.c $Rev: 153052 $");
