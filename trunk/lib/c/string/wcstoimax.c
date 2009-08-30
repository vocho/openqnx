/* wcstoimax function */
#include <inttypes.h>
#include "xwchar.h"
_STD_BEGIN

intmax_t (wcstoimax)(const wchar_t *_Restrict s,
	wchar_t **_Restrict endptr, int base)
	{	/* convert wide string to intmax_t, with checking */
	return (_WStoll(s, endptr, base));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcstoimax.c $Rev: 153052 $");
