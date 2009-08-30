/* strtold function */
#include <stdlib.h>
_STD_BEGIN

long double (strtold)(const char *_Restrict s, char **_Restrict endptr)
	{	/* convert string to long double, with checking */
	return (_Stold(s, endptr, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strtold.c $Rev: 153052 $");
