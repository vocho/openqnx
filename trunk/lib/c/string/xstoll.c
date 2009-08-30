/* _Stoll function */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "xmath.h"
#if defined(__WATCOMC__) && defined(__QNX__)
#include <stdint.h>
#endif

_STD_BEGIN

#if defined(__WATCOMC__) && defined(__QNX__)
#define MYMIN (-INTMAX_MAX - _C2) /* LLONG_MIN */
#define MYMAX INTMAX_MAX  /* LLONG_MAX */
#else
#define MYMIN	(-_LLONG_MAX - _C2)	/* LLONG_MIN */
#define MYMAX	_LLONG_MAX	/* LLONG_MAX */
#endif

_ULonglong _Stoullx(const char *, char **, int, int *);

_Longlong _Stollx(const char * s, char ** endptr,
	int base, int *perr)
	{	/* convert string to long long, with checking */
	const char *sc;
	char *se, sign;
	_ULonglong x;

	if (endptr == 0)
		endptr = &se;
	for (sc = s; isspace((unsigned char)*sc); ++sc)
		;
	sign = (char)(*sc == '-' || *sc == '+' ? *sc++ : '+');
	x = _Stoullx(sc, endptr, base, perr);
	if (sc == *endptr)
		*endptr = (char *)s;
	if (s == *endptr && x != 0 || sign == '+' && MYMAX < x
		|| sign == '-' && 0 - (_ULonglong)MYMIN < x)
		{	/* overflow */
		errno = ERANGE;
		if (perr != 0)
			*perr = 1;
		return (sign == '-' ? MYMIN : MYMAX);
		}
	else
		return ((_Longlong)(sign == '-' ? 0 - x : x));
	}

_Longlong (_Stoll)(const char * s, char ** endptr, int base)
	{	/* convert string, discard error code */
	return (_Stollx(s, endptr, base, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xstoll.c $Rev: 153052 $");
