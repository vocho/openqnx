/* wcstoull function */
#include "xwchar.h"
_STD_BEGIN

_ULonglong (wcstoull)(const wchar_t *_Restrict s,
	wchar_t **_Restrict endptr, int base)
	{	/* convert wide string to unsigned long long, with checking */
	return (_WStoull(s, endptr, base));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcstoull.c $Rev: 153052 $");
