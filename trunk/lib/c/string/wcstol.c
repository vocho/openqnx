/* wcstol function */
#include <errno.h>
#include <limits.h>
#include <wchar.h>
#include <wctype.h>
_STD_BEGIN

long (wcstol)(const wchar_t *s, wchar_t **endptr, int base)
	{	/* convert wide string to long, with checking */
	const wchar_t *sc;
	wchar_t *se, sign;
	unsigned long x;

	if (endptr == 0)
		endptr = &se;
	for (sc = s; iswspace(*sc); ++sc)
		;
	sign = *sc == L'-' || *sc == L'+' ? *sc++ : L'+';
	x = _WStoul(sc, endptr, base);
	if (sc == *endptr)
		*endptr = (wchar_t *)s;
	if (s == *endptr && x != 0
		|| sign == L'+' && LONG_MAX < x
		|| sign == L'-' && 0 - (unsigned long)LONG_MIN < x)
		{	/* overflow */
		errno = ERANGE;
		return (sign == L'-' ? LONG_MIN : LONG_MAX);
		}
	else
		return ((long)(sign == L'-' ? 0 - x : x));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcstol.c $Rev: 153052 $");
