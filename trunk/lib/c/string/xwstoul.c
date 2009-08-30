/*
 * $QNXtpLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */





/* _WStoul function */
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include "xwchar.h"

 #if (ULONG_MAX >> 16) >> 16 != 0xffffffff && ULONG_MAX != 0xffffffff
  #error LONGS TOO LARGE FOR _WStoul
 #endif /* longs too large */

_STD_BEGIN

		/* macros */
#define BASE_MAX	36	/* largest valid base */

		/* static data */
static const wchar_t digits[] = {	/* valid digits */
	L'0', L'1', L'2', L'3', L'4', L'5',
	L'6', L'7', L'8', L'9', L'a', L'b',
	L'c', L'd', L'e', L'f', L'g', L'h',
	L'i', L'j', L'k', L'l', L'm', L'n',
	L'o', L'p', L'q', L'r', L's', L't',
	L'u', L'v', L'w', L'x', L'y', L'z'};

 #if (ULONG_MAX >> 16) >> 16 == 0xffffffff
static const char ndigs[BASE_MAX + 1] = {	/* 64-bits! */
	0, 0, 65, 41, 33, 28, 25, 23, 22, 21,
	20, 19, 18, 18, 17, 17, 17, 16, 16, 16,
	15, 15, 15, 15, 14, 14, 14, 14, 14, 14,
	14, 13, 13, 13, 13, 13, 13,};

 #else /* (ULONG_MAX >> 16) >> 16 == 0xffffffff */
static const char ndigs[BASE_MAX+1] = {	/* 32-bits! */
	0, 0, 33, 21, 17, 14, 13, 12, 11, 11,
	10, 10, 9, 9, 9, 9, 9, 8, 8, 8,
	8, 8, 8, 8, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7,};
 #endif /* (ULONG_MAX >> 16) >> 16 == 0xffffffff */

unsigned long _WStoul(const wchar_t *s, wchar_t **endptr,
	int base)
	{	/* convert wide string to unsigned long */
	const wchar_t *sc, *sd;
	const wchar_t *s1, *s2;
	int dig = 0;
	wchar_t sign;
	ptrdiff_t n;
	unsigned long x, y = 0;

	for (sc = s; iswspace(*sc); ++sc)
		;
	sign = (wchar_t)(*sc == L'-' || *sc == L'+' ? *sc++ : L'+');
	if (base < 0 || base == 1 || BASE_MAX < base)
		{	/* silly base */
		if (endptr != 0)
			*endptr = (wchar_t *)s;
#ifdef __QNX__
		errno = EINVAL;
#endif
		return (0);
		}
	else if (0 < base)
		{	/* strip 0x or 0X */
		if (base == 16 && *sc == L'0'
			&& (sc[1] == L'x' || sc[1] == L'X'))
			sc += 2;
		}
	else if (*sc != L'0')
		base = 10;
	else if (sc[1] == L'x' || sc[1] == L'X')
		base = 16, sc += 2;
	else
		base = 8;
	for (s1 = sc; *sc == L'0'; ++sc)
		;	/* skip leading zeros */
	x = 0;
	for (s2 = sc; (sd = wmemchr(&digits[0], towlower(*sc), base))
		!= 0; ++sc)
		{	/* accumulate digits */
		y = x;
		dig = (int)(sd - digits);	/* for overflow checking */
		x = x * base + dig;
		}
	if (s1 == sc)
		{	/* check string validity */
		if (endptr != 0)
			*endptr = (wchar_t *)s;
		return (0);
		}
	n = (sc - s2) - ndigs[base];
	if (n < 0)
		;
	else if (0 < n || x < x - dig
		|| (x - dig) / (unsigned)base != y)
		{	/* overflow */
		errno = ERANGE;
		sc = s, x = ULONG_MAX, sign = L'+';
		}
	if (sign == L'-')	/* get final value */
		x = 0 - x;
	if (endptr != 0)
		*endptr = (wchar_t *)sc;
	return (x);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwstoul.c $Rev: 153052 $");
