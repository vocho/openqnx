/* _WPrintf function */
#include "xwstdio.h"
_STD_BEGIN

int _WPrintf(void *(*pfn)(void *, const wchar_t *, size_t),
	void *arg, const wchar_t *fmt, va_list ap_arg)
	{	/* print wide formatted */
	_WPft x;
	va_list ap;
	static const wchar_t percent[] = {L'%', L'\0'};

	_Vacopy(&ap, ap_arg);
	x.pfn = pfn, x.arg = arg, x.nchar = 0;
	for (; ; )
		{	/* scan format string */
		 {	/* put literal text */
		int n;

		if ((n = wcscspn(fmt, &percent[0])) <= 0)
			;
		else if ((x.arg = (*x.pfn)(x.arg, fmt, n)) == 0)
			return (EOF);
		else
			x.nchar += n;
		if (fmt[n] == L'\0')
			return (x.nchar);
		fmt += n + 1;
		 }
		 {	/* process a conversion specifier */
		const wchar_t *t;
		static const wchar_t fchar[] = {
			L' ', L'+', L'-', L'#', L'0',
			L',', L';', L':', L'_',	/* %v additions */
			L'\0'};
		static const wchar_t qchar[] = {
			L'h', L'j', L'l', L't',
			L'z', L'L', L'\0'};
		static const unsigned int fbit[] = {
			_FSP, _FPL, _FMI, _FNO, _FZE, 0};

 #define FBIT_SIZE	((sizeof (fbit) - 1) / sizeof (fbit[0]))

		x.n0 = x.nz0 = x.n1 = x.nz1 = x.n2 = x.nz2 = 0;

 #if _HAS_FIXED_POINT
		x.sep = L' ';
		for (x.flags = 0; (t = wcschr(&fchar[0], *fmt)) != 0; ++fmt)
			{	/* store either a flag or a %v separator */
			size_t idx = t - &fchar[0];

			if (idx < FBIT_SIZE)
				x.flags |= fbit[t - &fchar[0]];
			else
				x.sep = *fmt;
			}

 #else /* _HAS_FIXED_POINT */
		for (x.flags = 0; (t = wmemchr(&fchar[0], *fmt, FBIT_SIZE)) != 0;
			++fmt)
			x.flags |= fbit[t - &fchar[0]];
 #endif /* _HAS_FIXED_POINT */

		if (*fmt == L'*')
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
			for (x.width = 0; iswdigit(*fmt); ++fmt)
				if (x.width < _WMAX)
					x.width = x.width * 10 + *fmt - L'0';
		if (*fmt != L'.')
			x.prec = -1;
		else if (*++fmt == L'*')
			{	/* get precision argument */
			x.prec = va_arg(ap, int);
			++fmt;
			}
		else	/* accumulate precision digits */
			for (x.prec = 0; iswdigit(*fmt); ++fmt)
				if (x.prec < _WMAX)
					x.prec = x.prec * 10 + *fmt - L'0';

		x.qual = (wchar_t)(wcschr(&qchar[0], *fmt) != 0 ? *fmt++ : L'\0');

 #if _HAS_FIXED_POINT
		if (x.qual == L'\0' && *fmt == L'v')
			x.qual = *fmt++;	/* 'v', 'vh', or 'vl' */

		if (x.qual == L'h' && *fmt == L'v'
			|| x.qual == L'v' && *fmt == L'h')
			x.qual = L'w', ++fmt;	/* 'w' is 'hv' or 'vh' */
		else if (x.qual == L'l' && *fmt == L'v'
			|| x.qual == L'v' && *fmt == L'l')
			x.qual = L'W', ++fmt;	/* 'W' is 'lv' or 'vl' */
		else
 #endif /* _HAS_FIXED_POINT */

		if (x.qual == L'h' && *fmt == L'h')
			x.qual = L'b', ++fmt;
		else if (x.qual == L'l' && *fmt == L'l')
			x.qual = L'q', ++fmt;
		 }
		 {	/* do the conversion */
		wchar_t ac[_MAX_SIG_DIG + _MAX_EXP_DIG + 16];

		if (_WPutfld(&x, &ap, *fmt++, ac) < 0
			|| _WPuttxt(&x, ac) < 0)
			return (EOF);
		 }
		}
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwprintf.c $Rev: 153052 $");
