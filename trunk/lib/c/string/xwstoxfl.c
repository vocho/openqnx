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



/*




Also copyright P.J. Plauger - see bottom of file for details.
*/

/* _WStoxflt function */
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include "xmath.h"
_STD_BEGIN

#define BASE	16	/* hexadecimal */
#define NDIG	7	/* hexadecimal digits per long element */
#ifdef __QNX__
/* MAXSIG clashes with an unrelated MAXSIG in signal.h.
   So, undefine it. */
#undef MAXSIG
#endif
#define MAXSIG	(5 * NDIG)	/* maximum significant digits to keep */

int _WStoxflt(const wchar_t *s0, const wchar_t *s, wchar_t **endptr,
	long lo[], int maxsig)
	{	/* convert wide string to array of long plus exponent */
	char buf[MAXSIG + 1];	/* worst case, with room for rounding digit */
	int nsig;	/* number of significant digits seen */
	int seen;	/* any valid field characters seen */
	int word;	/* current long word to fill */

	const wchar_t *pd;
	static const wchar_t digits[] =
		{	/* hex digits in both cases */
		L'0', L'1', L'2', L'3',
		L'4', L'5', L'6', L'7',
		L'8', L'9', L'a', L'b',
		L'c', L'd', L'e', L'f',
		L'A', L'B', L'C', L'D',
		L'E', L'F', L'\0'};
	static const wchar_t vals[] =
		{	/* values of hex digits */
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		10, 11, 12, 13, 14, 15};

	maxsig *= NDIG;	/* convert word count to digit count */
	if (MAXSIG < maxsig)
		maxsig = MAXSIG;	/* protect against bad call */

	lo[0] = 0;	/* power of ten exponent */
	lo[1] = 0;	/* first NDIG-digit word of fraction */

	for (seen = 0; *s == L'0'; ++s, seen = 1)
		;	/* strip leading zeros */
	for (nsig = 0;
		(pd = (wchar_t *)wmemchr(&digits[0], *s, 22)) != 0; ++s, seen = 1)
#ifdef __QNX__
		/* Allow one digit more than maxsig, and round up later */
		if (nsig <= maxsig)
#else			
		if (nsig < maxsig)
#endif			
			buf[nsig++] = vals[pd - digits];	/* accumulate a digit */
		else
			++lo[0];	/* too many digits, just scale exponent */

	if (*s == localeconv()->decimal_point[0])
		++s;
	if (nsig == 0)
		for (; *s == '0'; ++s, seen = 1)
			--lo[0];	/* strip zeros after point */
	for (; (pd = (wchar_t *)wmemchr(&digits[0], *s, 22)) != 0; ++s, seen = 1)
		if (nsig <= maxsig)
			{	/* accumulate a fraction digit */
			buf[nsig++] = vals[pd - digits];
			--lo[0];
			}

	if (maxsig < nsig)
		{	/* discard excess digit after rounding up */
		unsigned int ms = maxsig;	/* to quiet warnings */

		if (BASE / 2 <= buf[ms])
			++buf[ms - 1];	/* okay if digit becomes BASE */
		nsig = maxsig;
		++lo[0];
		}
	for (; 0 < nsig && buf[nsig - 1] == '\0'; --nsig)
		++lo[0];	/* discard trailing zeros */
	if (nsig == 0)
		buf[nsig++] = '\0';	/* ensure at least one digit */
	lo[0] <<= 2;	/* change hex exponent to binary exponent */

	if (seen)
		{	/* convert digit sequence to words */
		int bufidx = 0;	/* next digit in buffer */
		int wordidx = NDIG - nsig % NDIG;	/* next digit in word (% NDIG) */

		word = wordidx % NDIG == 0 ? 0 : 1;
		for (; bufidx < nsig; ++wordidx, ++bufidx)
			if (wordidx % NDIG == 0)
				lo[++word] = buf[bufidx];
			else
				lo[word] = lo[word] * BASE + buf[bufidx];

		if (*s == L'p' || *s == L'P')
			{	/* parse exponent */
			const wchar_t *ssav = s;
			const wchar_t esign = *++s == L'+' || *s == L'-'
				? *s++ : L'+';
			int eseen = 0;
			long lexp = 0;

			for (; iswdigit(*s); ++s, eseen = 1)
				if (lexp < 100000000)	/* else overflow */
					lexp = lexp * 10 + *s - L'0';
			if (esign == '-')
				lexp = -lexp;
			lo[0] += lexp;
			if (!eseen)
				s = ssav;	/* roll back if incomplete exponent */
			}
		}

	if (!seen)
		word = 0;	/* return zero if bad parse */
	if (endptr)
		*endptr = (wchar_t *)(seen ? s : s0);	/* roll back if bad parse */
	return (word);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwstoxfl.c $Rev: 159979 $");

