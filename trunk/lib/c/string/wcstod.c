/* wcstod function */
#include <wchar.h>
_STD_BEGIN

double (wcstod)(const wchar_t *_Restrict s, wchar_t **_Restrict endptr)
	{	/* convert wide string to double, with checking */
	return (_WStod(s, endptr, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcstod.c $Rev: 153052 $");
