/* atof function */
#include <stdlib.h>
_STD_BEGIN

double (atof)(const char *s)
	{	/* convert string to double */
	return (_Stod(s, 0, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("atof.c $Rev: 153052 $");
