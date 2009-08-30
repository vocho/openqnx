/* strtol function */
#include <yvals.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "xmath.h"
_STD_BEGIN

long _Stolx(const char *, char **, int, int *);

long (strtol)(const char *_Restrict s, char **_Restrict endptr,
	int base)
	{	/* convert string, discard error code */
	return (_Stolx(s, endptr, base, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strtol.c $Rev: 153052 $");
