/* _WGetfloat function */
#include <locale.h>
#include "xmath.h"
#include "xwstdio.h"
_STD_BEGIN

int _WGetfloat(_WSft *px, void *pans)
	{	/* get a floating point value for _WScanf */
	wchar_t ac[_MAX_EXP_DIG + _MAX_SIG_DIG + 16], *p;
	wint_t ch;
	char seen;
	int nsig;
	int dlen, pten;
	static const wchar_t digits[] = {
		L'0', L'1', L'2', L'3',
		L'4', L'5', L'6', L'7',
		L'8', L'9', L'a', L'b',
		L'c', L'd', L'e', L'f',
		L'A', L'B', L'C', L'D',
		L'E', L'F', L'\0'};

	px->nget = 0 < px->width ? px->width : INT_MAX;
	p = ac, ch = WGETN(px);
	pten = 0;
	if (ch == L'+' || ch == L'-')
		*p++ = ch, ch = WGETN(px);
	dlen = 10;
	seen = 0;
	if (ch == L'0')
		{	/* match possible prefix and strip it */
		ch = WGETN(px);
		if (ch != L'x' && ch != L'X')
			seen = 1;
		else
			{	/* copy over "0x" and look for A format */
			*p++ = L'0';
			*p++ = L'x';
			ch = WGETN(px);
			dlen = 16 + 6;
			seen = 0;
			}
		}
	else if (ch == L'n' || ch == L'N')
		{	/* match "nan" */
		dlen = 0;
		*p++ = L'n', ch = WGETN(px);
		if (ch != L'a' && ch != L'A')
			WUNGETN(px, ch);
		else
			{	/* seen "na" */
			*p++ = L'a', ch = WGETN(px);
			if (ch != L'n' && ch != L'N')
				WUNGETN(px, ch);
			else if ((ch = WGETN(px)) != L'(')
				{	/* got "nan" not "nan(n-char-sequence)", quit */
				WUNGETN(px, ch);
				*p++ = L'n';
				seen = 1;
				}
			else
				{	/* parse (n-char-sequence) */
				for (; iswalnum(ch = WGETN(px)) || ch == L'_'; )
					;
				if (ch != L')')
					WUNGETN(px, ch);
				else
					{	/* got it, replace with "nan" */
					*p++ = L'n';
					seen = 1;
					}
				}
			}
		}
	else if (ch == L'i' || ch == L'I')
		{	/* match "inf" */
		dlen = 0;
		*p++ = L'i', ch = WGETN(px);
		if (ch != L'n' && ch != L'N')
			WUNGETN(px, ch);
		else
			{	/* seen "in" */
			*p++ = L'n', ch = WGETN(px);
			if (ch != L'f' && ch != L'F')
				WUNGETN(px, ch);
			else if ((ch = WGETN(px)) != L'i' && ch != L'I')
				{	/* got "inf" not "infinity", quit */
				WUNGETN(px, ch);
				*p++ = L'f';
				seen = 1;
				}
			else
				{	/* parse rest of "infinity" */
				if ((ch = WGETN(px)) != L'n' && ch != L'N'
					|| (ch = WGETN(px)) != L'i' && ch != L'I'
					|| (ch = WGETN(px)) != L't' && ch != L'T'
					|| (ch = WGETN(px)) != L'y' && ch != L'Y')
					WUNGETN(px, ch);
				else
					{	/* got it, replace with "inf" */
					*p++ = L'f';
					seen = 1;
					}
				}
			}
		}
	if (0 < dlen)
		{	/* match rest of hex or decimal field */
		for (; ch == L'0'; seen = 1)
			ch = WGETN(px);	/* strip leading zeros */
		if (seen)
			*p++ = L'0';	/* put one back */
		for (nsig = 0; ch != WEOF && wmemchr(&digits[0], ch, dlen);
			ch = WGETN(px), seen = 1)
			if (nsig < _MAX_SIG_DIG)
				*p++ = ch, ++nsig;
			else
				++pten;
		if (_Wctob(ch) == localeconv()->decimal_point[0])
			*p++ = ch, ch = WGETN(px);
		if (nsig == 0)
			{	/* strip zeros after point */
			for (; ch == L'0'; ch = WGETN(px), seen = 1)
				--pten;
			if (pten < 0)
				*p++ = L'0', ++pten;	/* put one back */
			}
		for (; ch != WEOF && wmemchr(&digits[0], ch, dlen);
			ch = WGETN(px), seen = 1)
			if (nsig < _MAX_SIG_DIG)
				*p++ = ch, ++nsig;
		if (seen && (dlen == 10 && (ch == L'e' || ch == L'E')
			|| dlen != 10 && (ch == L'p' || ch == L'P')))
			{	/* parse exponent */
			*p++ = ch, ch = WGETN(px);
			if (ch == L'+' || ch == L'-')
				*p++ = ch, ch = WGETN(px);
			for (seen = 0; ch == L'0'; ch = WGETN(px), seen = 1)
				;	/* strip leading exponent zeros */
			if (seen)
				*p++ = L'0';	/* put one back */
			for (nsig = 0; iswdigit(ch); ch = WGETN(px), seen = 1)
				if (nsig < _MAX_EXP_DIG)
					*p++ = ch, ++nsig;
			}
		WUNGETN(px, ch);
		}
	if (!seen)
		return (p == ac && ch == WEOF ? EOF : 0);
	*p = L'\0';
	if (!px->noconv)
		{	/* convert and store */
		long double ldval;

		if (dlen <= 10)
			ldval = _WStold(ac, 0, pten);
		else
			{	/* convert fraction and scale by hex exponent */
			ldval = _WStold(ac, 0, 0);
			_LDscale(&ldval, pten * 4);
			}
		px->stored = 1;
		if (px->qual == L'l')
			if (pans != 0)
				*(double *)pans = (double)ldval;
			else
				*va_arg(px->ap, double *) = (double)ldval;
		else if (px->qual != L'L')
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

__SRCVERSION("xwgetflo.c $Rev: 153052 $");
