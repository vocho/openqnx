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





/* _Stoul function */
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include "xmath.h"

 #if (ULONG_MAX >> 16) >> 16 != 0xffffffff && ULONG_MAX != 0xffffffff
  #error LONGS TOO LARGE FOR _Stoul
 #endif /* longs too large */

_STD_BEGIN

		/* macros */
#define BASE_MAX	36	/* largest valid base */

		/* static data */
static const char digits[] = 	/* valid digits */
	"0123456789abcdefghijklmnopqrstuvwxyz";

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

unsigned long _Stoulx(const char *s, char **endptr,
	int base, int *perr)
	{	/* convert string to unsigned long, with checking */
	const char *sc, *sd;
	const char *s1, *s2;
	char dig = 0, sign;
	ptrdiff_t n;
	unsigned long x, y = 0;

	if (perr != 0)
		*perr = 0;
	for (sc = s; isspace((unsigned char)*sc); ++sc)
		;
	sign = *sc == '-' || *sc == '+' ? *sc++ : '+';
	if (base < 0 || base == 1 || BASE_MAX < base)
		{	/* silly base */
		if (endptr != 0)
			*endptr = (char *)s;
#ifdef __QNX__
		errno = EINVAL;
#endif
		return (0);
		}
	else if (0 < base)
		{	/* strip 0x or 0X */
		if (base == 16 && *sc == '0'
			&& (sc[1] == 'x' || sc[1] == 'X'))
			sc += 2;
		}
	else if (*sc != '0')
		base = 10;
	else if (sc[1] == 'x' || sc[1] == 'X')
		base = 16, sc += 2;
	else
		base = 8;
	for (s1 = sc; *sc == '0'; ++sc)
		;	/* skip leading zeros */
	x = 0;
	for (s2 = sc; (sd = (char *)memchr(&digits[0],
		tolower(*sc), base)) != 0; ++sc)
		{	/* accumulate digits */
		y = x;
		dig = (char)(sd - digits);	/* for overflow checking */
		x = x * base + dig;
		}
	if (s1 == sc)
		{	/* check string validity */
		if (endptr != 0)
			*endptr = (char *)s;
		return (0);
		}
	n = (sc - s2) - ndigs[base];
	if (n < 0)
		;
	else if (0 < n || x < x - dig
		|| (x - dig) / (unsigned)base != y)
		{	/* overflow */
		errno = ERANGE;
		if (perr != 0)
			*perr = 1;
		x = ULONG_MAX, sign = '+';
		}
	if (sign == '-')	/* get final value */
		x = 0 - x;
	if (endptr != 0)
		*endptr = (char *)sc;
	return (x);
	}

unsigned long _Stoul(const char *s, char **endptr, int base)
	{	/* convert string, discard error code */
	return (_Stoulx(s, endptr, base, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xstoul.c $Rev: 153052 $");
