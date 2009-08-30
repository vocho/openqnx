/* wcslen function */
#include <wchar.h>
_STD_BEGIN

size_t (wcslen)(const wchar_t *s)
	{	/* find length of wchar_t s[] */
	const wchar_t *sc;

	for (sc = s; *sc != L'\0'; ++sc)
		;
	return (sc - s);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcslen.c $Rev: 153052 $");
