/* strtof function */
#include <stdlib.h>
_STD_BEGIN

float (strtof)(const char *_Restrict s, char **_Restrict endptr)
	{	/* convert string to float, with checking */
	return (_Stof(s, endptr, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strtof.c $Rev: 153052 $");
