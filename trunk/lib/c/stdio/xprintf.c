/* _Printf function */
#include <ctype.h>
#include <string.h>
#include "xstdio.h"
#include "xwchar.h"
_STD_BEGIN

 #if __EDG__	/* compiler test */
#undef stderr
extern FILE _Stderr;
FILE *stderr = &_Stderr;	/* displace any other stderr */
 #endif /* __EDG__ */

int _Printf(void *(*pfn)(void *, const char *, size_t),
	void *arg, const char *fmt, va_list ap_arg)
	{	/* print formatted */
	_Pft x;
	_Mbstinit(mbst);
	va_list ap;

	_Vacopy(&ap, ap_arg);
	x.pfn = pfn, x.arg = arg, x.nchar = 0;
	for (; ; )
		{	/* scan format string */
		for (; ; )
			{	/* copy any literal text */
			int m, n;
			wchar_t wc = L'\0';

			if ((n = _Mbtowc(&wc, fmt, INT_MAX, &mbst)) <= 0)
				n = *fmt == '\0' ? 0 : 1;	/* bad parse, eat one char */
			if ((m = (wc == L'%' ? n - 1 : n)) <= 0)
				;
			else if ((x.arg = (*x.pfn)(x.arg, fmt, m)) == 0)
				return (EOF);
			else
				x.nchar += m;
			fmt += n;
			if (wc == L'%')
				break;
			else if (wc == L'\0')
				return (x.nchar);
			}
		 {	/* process a conversion specifier */
		const char *t;
		static const char fchar[] = " +-#0" /* %v additions */ ",;:_";
		static const unsigned int fbit[] = {
			_FSP, _FPL, _FMI, _FNO, _FZE, 0};

 #define FBIT_SIZE	((sizeof (fbit) - 1) / sizeof (fbit[0]))

		x.n0 = x.nz0 = x.n1 = x.nz1 = x.n2 = x.nz2 = 0;

 #if _HAS_FIXED_POINT
		x.sep = ' ';
		for (x.flags = 0; (t = strchr(&fchar[0], *fmt)) != 0; ++fmt)
			{	/* store either a flag or a %v separator */
			size_t idx = t - &fchar[0];

			if (idx < FBIT_SIZE)
				x.flags |= fbit[t - &fchar[0]];
			else
				x.sep = *fmt;
			}

 #else /* _HAS_FIXED_POINT */
		for (x.flags = 0; (t = memchr(&fchar[0], *fmt, FBIT_SIZE)) != 0;
			++fmt)
			x.flags |= fbit[t - &fchar[0]];
 #endif /* _HAS_FIXED_POINT */

		if (*fmt == '*')
			{	/* get width argument */
			x.width = va_arg(ap, int);
			if (x.width < 0)
				{	/* same as '-' flag */
				x.width = -x.width;
				x.flags |= _FMI;
				}
			++fmt;
			}
		else	/* accumulate width digits */
			for (x.width = 0; isdigit((unsigned char)*fmt); ++fmt)
				if (x.width < _WMAX)
					x.width = x.width * 10 + (unsigned char)*fmt - '0';
		if (*fmt != '.')
			x.prec = -1;
		else if (*++fmt == '*')
			{	/* get precision argument */
			x.prec = va_arg(ap, int);
			++fmt;
			}
		else	/* accumulate precision digits */
			for (x.prec = 0; isdigit((unsigned char)*fmt); ++fmt)
				if (x.prec < _WMAX)
					x.prec = x.prec * 10 + (unsigned char)*fmt - '0';

#ifdef __QNX__
		x.qual = (char)(strchr("hjltzLq", *fmt) ? *fmt++ : '\0');
#else
		x.qual = (char)(strchr("hjltzL", *fmt) ? *fmt++ : '\0');
#endif

 #if _HAS_FIXED_POINT
		if (x.qual == '\0' && *fmt == 'v')
			x.qual = *fmt++;	/* 'v', 'vh', or 'vl' */

		if (x.qual == 'h' && *fmt == 'v'
			|| x.qual == 'v' && *fmt == 'h')
			x.qual = 'w', ++fmt;	/* 'w' is 'hv' or 'vh' */
		else if (x.qual == 'l' && *fmt == 'v'
			|| x.qual == 'v' && *fmt == 'l')
			x.qual = 'W', ++fmt;	/* 'W' is 'lv' or 'vl' */
		else
 #endif /* _HAS_FIXED_POINT */

		if (x.qual == 'h' && *fmt == 'h')
			x.qual = 'b', ++fmt;
		else if (x.qual == 'l' && *fmt == 'l')
			x.qual = 'q', ++fmt;
		 }
		 {	/* do the conversion */
		char ac[_MAX_SIG_DIG + _MAX_EXP_DIG + 16];

		if (_Putfld(&x, &ap, *fmt++, ac) || _Puttxt(&x, ac) < 0)
			return (EOF);
		 }
		}
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xprintf.c $Rev: 153052 $");
