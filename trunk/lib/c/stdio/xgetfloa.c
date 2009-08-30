/* _Getfloat function */
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "xmath.h"
#include "xstdio.h"
_STD_BEGIN

int _Getfloat(_Sft *px, void *pans)
	{	/* get a floating point value for _Scanf */
	char ac[_MAX_EXP_DIG + _MAX_SIG_DIG + 16], *p, seen;
	int ch, nsig;
	int dlen, pten;
	static const char digits[] = "0123456789abcdefABCDEF";

	px->nget = 0 < px->width ? px->width : INT_MAX;
	p = ac, ch = GETN(px);
	pten = 0;
	if (ch == '+' || ch == '-')
		*p++ = ch, ch = GETN(px);
	dlen = 10;
	seen = 0;
	if (ch == '0')
		{	/* match possible prefix and strip it */
		ch = GETN(px);
		if (ch != 'x' && ch != 'X')
			seen = 1;
		else
			{	/* copy over "0x" and look for A format */
			*p++ = '0';
			*p++ = 'x';
			ch = GETN(px);
			dlen = 16 + 6;
			seen = 0;
			}
		}
	else if (ch == 'n' || ch == 'N')
		{	/* match "nan" */
		dlen = 0;
		*p++ = 'n', ch = GETN(px);
		if (ch != 'a' && ch != 'A')
			UNGETN(px, ch);
		else
			{	/* seen "na" */
			*p++ = 'a', ch = GETN(px);
			if (ch != 'n' && ch != 'N')
				UNGETN(px, ch);
			else if ((ch = GETN(px)) != '(')
				{	/* got "nan" not "nan(n-char-sequence)", quit */
				UNGETN(px, ch);
				*p++ = 'n';
				seen = 1;
				}
			else
				{	/* parse (n-char-sequence) */
				for (; isalnum(ch = GETN(px)) || ch == '_'; )
					;
				if (ch != ')')
					UNGETN(px, ch);
				else
					{	/* got it, replace with "nan" */
					*p++ = 'n';
					seen = 1;
					}
				}
			}
		}
	else if (ch == 'i' || ch == 'I')
		{	/* match "inf" */
		dlen = 0;
		*p++ = 'i', ch = GETN(px);
		if (ch != 'n' && ch != 'N')
			UNGETN(px, ch);
		else
			{	/* seen "in" */
			*p++ = 'n', ch = GETN(px);
			if (ch != 'f' && ch != 'F')
				UNGETN(px, ch);
			else if ((ch = GETN(px)) != 'i' && ch != 'I')
				{	/* got "inf" not "infinity", quit */
				UNGETN(px, ch);
				*p++ = 'f';
				seen = 1;
				}
			else
				{	/* parse rest of "infinity" */
				if ((ch = GETN(px)) != 'n' && ch != 'N'
					|| (ch = GETN(px)) != 'i' && ch != 'I'
					|| (ch = GETN(px)) != 't' && ch != 'T'
					|| (ch = GETN(px)) != 'y' && ch != 'Y')
					UNGETN(px, ch);
				else
					{	/* got it, replace with "inf" */
					*p++ = 'f';
					seen = 1;
					}
				}
			}
		}
	if (0 < dlen)
		{	/* match rest of hex or decimal field */
		for (; ch == '0'; seen = 1)
			ch = GETN(px);	/* strip leading zeros */
		if (seen)
			*p++ = '0';	/* put one back */
		for (nsig = 0; ch != EOF && memchr(&digits[0], ch, dlen);
			ch = GETN(px), seen = 1)
			if (nsig < _MAX_SIG_DIG)
				*p++ = ch, ++nsig;
			else
				++pten;
		if (ch == localeconv()->decimal_point[0])
			*p++ = ch, ch = GETN(px);
		if (nsig == 0)
			{	/* strip zeros after point */
			for (; ch == '0'; ch = GETN(px), seen = 1)
				--pten;
			if (pten < 0)
				*p++ = '0', ++pten;	/* put one back */
			}
		for (; ch != EOF && memchr(&digits[0], ch, dlen);
			ch = GETN(px), seen = 1)
			if (nsig < _MAX_SIG_DIG)
				*p++ = ch, ++nsig;
		if (seen && (dlen == 10 && (ch == 'e' || ch == 'E')
			|| dlen != 10 && (ch == 'p' || ch == 'P')))
			{	/* parse exponent */
			*p++ = ch, ch = GETN(px);
			if (ch == '+' || ch == '-')
				*p++ = ch, ch = GETN(px);
			for (seen = 0; ch == '0'; ch = GETN(px), seen = 1)
				;	/* strip leading exponent zeros */
			if (seen)
				*p++ = '0';	/* put one back */
			for (nsig = 0; isdigit(ch); ch = GETN(px), seen = 1)
				if (nsig < _MAX_EXP_DIG)
					*p++ = ch, ++nsig;
			}
		UNGETN(px, ch);
		}
	if (!seen)
		return (p == ac && ch == EOF ? EOF : 0);
	*p = '\0';
	if (!px->noconv)
		{	/* convert and store */
		long double ldval;

		if (dlen <= 10)
			ldval = _Stold(ac, 0, pten);
		else
			{	/* convert fraction and scale by hex exponent */
			ldval = _Stold(ac, 0, 0);
			_LDscale(&ldval, pten * 4);
			}

		px->stored = 1;
		if (px->qual == 'l')
			if (pans != 0)
				*(double *)pans = (double)ldval;
			else
				*va_arg(px->ap, double *) = (double)ldval;
		else if (px->qual != 'L')
			if (pans != 0)
				*(float *)pans = (float)ldval;
			else
				*va_arg(px->ap, float *) = (float)ldval;
		else
			if (pans != 0)
				*(long double *)pans = ldval;
			else
				*va_arg(px->ap, long double *) = ldval;
		}
	return (1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xgetfloa.c $Rev: 153052 $");
