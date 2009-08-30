/* _Scanf function */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "xstdio.h"
#include "xwchar.h"
_STD_BEGIN

int _Scanf(int (*pfn)(void *, int, int), void *arg,
	const char *fmt, va_list ap, int secure)
	{	/* read formatted */
	int nconv = -1;
	_Sft x;
	_Mbstinit(mbst);

	x.pfn = pfn;
	x.arg = arg;
	_Vacopy(&x.ap, ap);
	x.secure = (char)secure;
	x.nchar = 0;
	for (x.s = fmt; ; ++x.s)
		{	/* parse format string */
		int ch;

		for (; ; )
			{	/* match any literal text or white-space */
			int n;
			wchar_t wc = L'\0';

			if ((n = _Mbtowc(&wc, x.s, INT_MAX, &mbst)) <= 0)
				n = *x.s == '\0' ? 0 : 1;	/* bad parse, eat one char */
			if (isspace(_Wctob(wc)))
				{	/* match any white-space */
				while (isspace(ch = GET(&x)))
					;
				UNGETN(&x, ch);
				if (ch == EOF)
					return (nconv < 0 ? EOF : nconv);
				}
			else
				{	/* match any literal text */
				int m = wc == L'%' ? n - 1 : n;
				const char *s;

				for (s = x.s; 0 <= --m; ++s)
					if ((ch = GET(&x)) != *s)
						{	/* bad match */
						UNGETN(&x, ch);
#ifdef __QNX__
						return (nconv < 0 ? (ch == EOF ? EOF : 0) : nconv);
#else
						return (nconv < 0 ? 0 : nconv);
#endif
						}
				}
			x.s += n;
			if (wc == L'%')
				break;
			else if (wc == L'\0')
				return (nconv < 0 ? 0 : nconv);
			}
		 {	/* process a conversion specifier */
		int code;

 #if _HAS_FIXED_POINT
		const char *t;
		static const char fchar[] = "*" /* %v additions */ ",;:_";

		x.sep = ' ';
		for (x.noconv = 0; (t = strchr(&fchar[0], *x.s)) != 0; ++x.s)
			{	/* store either '*' in noconv or a %v separator */
			if (&fchar[0] == t)
				x.noconv = '*';
			else
				x.sep = *x.s;
			}

 #else /* _HAS_FIXED_POINT */
		x.noconv = (char)(*x.s == '*' ? *x.s++ : '\0');
 #endif /* _HAS_FIXED_POINT */

		for (x.width = 0; isdigit((unsigned char)*x.s); ++x.s)
			if (x.width < _WMAX)
				x.width = x.width * 10 + (unsigned char)*x.s - '0';

/* EXTENSION for {c s []} array size */
		if (*x.s != '.')
			x.prec = x.secure ? (size_t)0 : (size_t)(-1);
		else if (*++x.s == '*')
			{	/* get precision argument */
			x.prec = va_arg(ap, size_t);
			++x.s;
			}
		else
			{	/* accumulate precision digits */
			for (x.prec = 0; isdigit((unsigned char)*x.s); ++x.s)
				if (x.prec <= (size_t)(-1) - ((unsigned char)*x.s - '0'))
					x.prec = x.prec * 10 + (unsigned char)*x.s - '0';
				else
					x.prec = (size_t)(-1);	/* remember overflow */
			if (x.secure || x.prec == (size_t)(-1))
				x.prec = 0;	/* make secure, overflow an invalid precision */
			}

		x.qual = (char)(strchr("hjltzL", *x.s) ? *x.s++ : '\0');

 #if _HAS_FIXED_POINT
		if (x.qual == '\0' && *x.s == 'v')
			x.qual = *x.s++;	/* 'v', 'vh', or 'vl' */

		if (x.qual == 'h' && *x.s == 'v'
			|| x.qual == 'v' && *x.s == 'h')
			x.qual = 'w', ++x.s;	/* 'w' is 'hv' or 'vh' */
		else if (x.qual == 'l' && *x.s == 'v'
			|| x.qual == 'v' && *x.s == 'l')
			x.qual = 'W', ++x.s;	/* 'W' is 'lv' or 'vl' */
		else
 #endif /* _HAS_FIXED_POINT */

		if (x.qual == 'h' && *x.s == 'h')
			x.qual = 'b', ++x.s;
		else if (x.qual == 'l' && *x.s == 'l')
			x.qual = 'q', ++x.s;
		if (!strchr("cCn[", *x.s))
			{	/* match leading white-space */
			while (isspace(ch = GET(&x)))
				;
			UNGETN(&x, ch);
#ifdef __QNX__
			if (ch == EOF)
				return (nconv < 0 ? EOF : nconv);
#endif
			}
		if ((code = _Getfld(&x)) <= 0)
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

__SRCVERSION("xscanf.c $Rev: 153052 $");
