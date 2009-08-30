/* _WScanf function */
#include <string.h>
#include "xwstdio.h"
_STD_BEGIN

int _WScanf(wint_t (*pfn)(void *, wint_t, int), void *arg,
	const wchar_t *fmt, va_list ap, int secure)
	{	/* read formatted */
	int nconv = -1;
	_WSft x;

	x.pfn = pfn;
	x.arg = arg;
	_Vacopy(&x.ap, ap);
	x.secure = (char)secure;
	x.nchar = 0;
	for (x.s = fmt; ; ++x.s)
		{	/* parse format string */
		wint_t ch;

		for (; *x.s != L'%'; ++x.s)
			if (*x.s == L'\0')
				return (nconv < 0 ? 0 : nconv);
			else if (iswspace(*x.s))
				{	/* match any white-space */
				while (iswspace(ch = WGET(&x)))
					;
				WUNGETN(&x, ch);
				if (ch == WEOF)
					return (nconv < 0 ? EOF : nconv);
				}
			else if (*x.s != (ch = WGET(&x)))
				{	/* bad literal match */
				WUNGETN(&x, ch);
				return (nconv < 0 ? 0 : nconv);
				}
		 {	/* process a conversion specifier */
		int code;
		static const wchar_t qchar[] = {
			L'h', L'j', L'l', L't',
			L'z', L'L', L'\0'};
		static const wchar_t schar[] = {
			L'c', L'C', L'n', L'[', L'\0'};

 #if _HAS_FIXED_POINT
		const wchar_t *t;
		static const wchar_t fchar[] = {
			L'*',
			L',', L';', L':', L'_', L'\0'};

		x.sep = ' ';
		for (x.noconv = 0; (t = wcschr(&fchar[0], *++x.s)) != 0; )
			{	/* store either '*' in noconv or a %v separator */
			if (&fchar[0] == t)
				x.noconv = L'*';
			else
				x.sep = *x.s;
			}

 #else /* _HAS_FIXED_POINT */
		if (*++x.s == L'*')
			x.noconv = '*', ++x.s;
		else
			x.noconv = '\0';
 #endif /* _HAS_FIXED_POINT */

		for (x.width = 0; iswdigit(*x.s); ++x.s)
			if (x.width < _WMAX)
				x.width = x.width * 10 + *x.s - L'0';

/* EXTENSION for {c s []} array size */
		if (*x.s != L'.')
			x.prec = x.secure ? (size_t)0 : (size_t)(-1);
		else if (*++x.s == L'*')
			{	/* get precision argument */
			x.prec = va_arg(ap, size_t);
			++x.s;
			}
		else
			{	/* accumulate precision digits */
			for (x.prec = 0; iswdigit(*x.s); ++x.s)
				if (x.prec <= (size_t)(-1) - (*x.s - L'0'))
					x.prec = x.prec * 10 + *x.s - L'0';
				else
					x.prec = (size_t)(-1);	/* remember overflow */
			if (x.secure || x.prec == (size_t)(-1))
				x.prec = 0;	/* make overflow an invalid precision */
			}

		x.qual = (wchar_t)(wcschr(&qchar[0], *x.s) ? *x.s++ : L'\0');

 #if _HAS_FIXED_POINT
		if (x.qual == L'\0' && *x.s == L'v')
			x.qual = *x.s++;	/* 'v', 'vh', or 'vl' */

		if (x.qual == L'h' && *x.s == L'v'
			|| x.qual == L'v' && *x.s == L'h')
			x.qual = L'w', ++x.s;	/* 'w' is 'hv' or 'vh' */
		else if (x.qual == L'l' && *x.s == L'v'
			|| x.qual == L'v' && *x.s == L'l')
			x.qual = L'W', ++x.s;	/* 'W' is 'lv' or 'vl' */
		else
 #endif /* _HAS_FIXED_POINT */

		if (x.qual == L'h' && *x.s == L'h')
			x.qual = L'b', ++x.s;
		else if (x.qual == L'l' && *x.s == L'l')
			x.qual = L'q', ++x.s;
		if (!wcschr(&schar[0], *x.s))
			{	/* match leading white-space */
			while (iswspace(ch = WGET(&x)))
				;
			WUNGETN(&x, ch);
			}
		if ((code = _WGetfld(&x)) <= 0)
			return (nconv < 0 ? code : nconv);
		if (nconv < 0)
			nconv = 0;
		if (x.stored)
			++nconv;
		 }
		}
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwscanf.c $Rev: 153052 $");
