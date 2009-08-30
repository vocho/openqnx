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





/* _Ldtob function */
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "xmath.h"
#include "xstdio.h"
_STD_BEGIN

		/* macros */
#define MAXDIG	40	/* safe for 128-bit long double */
#define NDIG	8	/* decimal digits generated for each multiply */
#define NXDIG	7	/* hexadecimal digits generated for each multiply */

		/* static data */
static const long double pows[] = {
	1e1L, 1e2L, 1e4L, 1e8L, 1e16L, 1e32L,

 #if 0x100 < _LBIAS	/* assume IEEE 754 8- or 10-byte */
	1e64L, 1e128L, 1e256L,

  #if _DLONG			/* assume IEEE 754 10-byte */
	1e512L, 1e1024L, 1e2048L, 1e4096L,
  #endif /* _DLONG */

 #endif /* 0x100 < _LBIAS */
	};

void _Ldtob(_Pft *px, char code)
	{	/* convert long double to text */
	char ac[MAXDIG];
	char *p = ac;
	long double ldval = px->v.ld;
	short errx, nsig, xexp;

	if (code == 'a' || code == 'A')
		;
	else if (px->prec < 0)
		px->prec = 6;
	else if (px->prec == 0 && (code == 'g' || code == 'G'))
		px->prec = 1;
	if ((errx = _LDunscale(&xexp, &px->v.ld)) == _NANCODE)
		{	/* x == NaN */
		memcpy(px->s, code == 'a' || code == 'e' || code == 'f' || code == 'g'
			? "nan" : "NAN", px->n1 = 3);
		return;
		}
	else if (0 < errx)
		{	/* x == INF */
		memcpy(px->s, code == 'a' || code == 'e' || code == 'f' || code == 'g'
			? "inf" : "INF", px->n1 = 3);
		return;
		}

	if (code == 'a' || code == 'A')
		{	/* put "0x" */
		*px->s++ = '0';
		*px->s++ = code == 'a' ? 'x' : 'X';
		px->n0 +=2;
		}

	if (0 == errx)	/* x == 0 */
		nsig = 0, xexp = 0;
	else if (code == 'a' || code == 'A')
		{	/* 0 < |x|, generate hex fraction, binary exponent */
		const char *digits = code == 'a'
			? "0123456789abcdef" : "0123456789ABCDEF";
		int gen;

		nsig = px->prec < 0 ? MAXDIG - NXDIG : px->prec + 1;
		gen = nsig + 2;
		ldval = ldval < 0 ? -px->v.ld : px->v.ld;
		xexp -= 4;	/* one leading nonzero hex digit */

		for (*p++ = 0x0; 0 < gen && 0 < ldval; p += NXDIG)
			{	/* convert NXDIG at a time */
			int j;
			long lo;

			_LDscale(&ldval, 4 * NXDIG);
			lo = (long)ldval;
			if (0 < (gen -= NXDIG))
				ldval -= (long double)lo;
			for (p += NXDIG, j = NXDIG; 0 < lo && 0 <= --j; )
				*--p = (int)(lo & 0xf), lo >>= 4;
			while (0 <= --j)
				*--p = 0;
			}
		gen = p - &ac[1];
		p = &ac[1];
		if (gen < nsig)
			nsig = gen;
		if (0 <= nsig)
			{	/* round and strip trailing zeros */
			const char drop = nsig < gen && 0x8 <= p[nsig] ? 0xf : 0x0;
			int n;

			for (n = nsig; p[--n] == drop; )
				--nsig;
			if (drop == 0xf)
				++p[n];
			if (n < 0)
				--p, ++nsig, xexp += 4;
			for (n = nsig; 0 <= --n; )
				p[n] = digits[p[n]];
			}
		if (px->prec < 0)
			px->prec = nsig - 1;
		}
	else
		{	/* 0 < |x|, generate decimal fraction and exponent */
		 {	/* scale ldval to ~~10^(NDIG/2) */
		int i;
		unsigned n;

		if (ldval < 0)
			ldval = -ldval;
		if ((xexp = xexp * 30103L / 100000L - NDIG / 2) < 0)
			{	/* scale up */
			n = (-xexp + (NDIG / 2 - 1)) & ~(NDIG / 2 - 1);
			xexp = -n;
			for (i = 0; 0 < n; n >>= 1, ++i)
				if (n & 1)
					ldval *= pows[i];
			}
		else if (0 < xexp)
			{	/* scale down */
			long double factor = 1.0;

			xexp &= ~(NDIG / 2 - 1);
			for (n = xexp, i = 0; 0 < n; n >>= 1, ++i)
				if (n & 1)
					factor *= pows[i];
			ldval /= factor;
			}
		 }
		 {	/* convert significant digits */
		int gen = px->prec + (code == 'f' || code == 'F'
			? xexp + 3 + NDIG : 3 + NDIG / 2);

		if (LDBL_DIG + NDIG / 2 < gen)
			gen = LDBL_DIG + NDIG / 2;
		for (*p++ = '0'; 0 < gen && 0 < ldval; p += NDIG)
			{	/* convert NDIG at a time */
			int j;
			long lo = (long)ldval;

			if (0 < (gen -= NDIG))
				ldval = (ldval - (long double)lo) * 1e8L;
			for (p += NDIG, j = NDIG; 0 < lo && 0 <= --j; )
				{	/* convert NDIG digits */
				ldiv_t qr = ldiv(lo, 10);

				*--p = qr.rem + '0', lo = qr.quot;
				}
			while (0 <= --j)
				*--p = '0';
			}
		gen = p - &ac[1];
		for (p = &ac[1], xexp += NDIG - 1; *p == '0'; ++p)
			--gen, --xexp;	/* correct xexp */
		nsig = px->prec + (code == 'f' || code == 'F'
			? xexp + 1 : code == 'e' || code == 'E' ? 1 : 0);
		if (gen < nsig)
			nsig = gen;
		if (0 <= nsig)
			{	/* round and strip trailing zeros */
			const char drop = nsig < gen && '5' <= p[nsig] ? '9' : '0';
			int n;

			for (n = nsig; p[--n] == drop; )
				--nsig;
			if (drop == '9')
				++p[n];
			if (n < 0)
				--p, ++nsig, ++xexp;
			}
		 }
		}
	_Genld(px, code, p, nsig, xexp);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xldtob.c $Rev: 153052 $");
