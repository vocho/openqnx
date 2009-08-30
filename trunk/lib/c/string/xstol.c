/* _Stolx function */
#include <yvals.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "xmath.h"
_STD_BEGIN

unsigned long _Stoulx(const char *, char **, int, int *);

long _Stolx(const char * s, char ** endptr,
	int base, int *perr)
	{	/* convert string to long, with checking */
	const char *sc;
	char *se, sign;
	unsigned long x;

	if (endptr == 0)
		endptr = &se;
	for (sc = s; isspace((unsigned char)*sc); ++sc)
		;
	sign = *sc == '-' || *sc == '+' ? *sc++ : '+';
	x = _Stoulx(sc, endptr, base, perr);
	if (sc == *endptr)
		*endptr = (char *)s;
	if (s == *endptr && x != 0 || sign == '+' && LONG_MAX < x
		|| sign == '-' && 0 - (unsigned long)LONG_MIN < x)
		{	/* overflow */
		errno = ERANGE;
		if (perr != 0)
			*perr = 1;
		return (sign == '-' ? LONG_MIN : LONG_MAX);
		}
	else
		return ((long)(sign == '-' ? 0 - x : x));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xstol.c $Rev: 153052 $");
