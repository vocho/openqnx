/* wcstoul function */
#include <wchar.h>
_STD_BEGIN

unsigned long (wcstoul)(const wchar_t *s, wchar_t **endptr,
	int base)
	{	/* convert wide string to unsigned long */
	return (_WStoul(s, endptr, base));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcstoul.c $Rev: 153052 $");
