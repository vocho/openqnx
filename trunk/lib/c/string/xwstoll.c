/* _Wstoll function */
#include <errno.h>
#include <limits.h>
#include <wctype.h>
#include "xwchar.h"
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

_Longlong (_WStoll)(const wchar_t * s, wchar_t **endptr, int base)
	{	/* convert wide string to long long, with checking */
	const wchar_t *sc;
	wchar_t *se, sign;
	_ULonglong x;

	if (endptr == 0)
		endptr = &se;
	for (sc = s; iswspace(*sc); ++sc)
		;
	sign = (wchar_t)(*sc == L'-' || *sc == L'+' ? *sc++ : L'+');
	x = _WStoull(sc, endptr, base);
	if (sc == *endptr)
		*endptr = (wchar_t *)s;
	if (s == *endptr && x != 0
		|| sign == L'+' && MYMAX < x
		|| sign == L'-' && 0 - (_ULonglong)MYMIN < x)
		{	/* overflow */
		errno = ERANGE;
		return (sign == L'-' ? MYMIN : MYMAX);
		}
	else
		return ((_Longlong)(sign == L'-' ? 0 - x : x));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwstoll.c $Rev: 153052 $");
