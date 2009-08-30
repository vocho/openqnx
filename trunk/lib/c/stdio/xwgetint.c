/* _WGetint function */
#include "xwstdio.h"
_STD_BEGIN

#define ACSIZE	32	/* holds only prefix, m.s. digits */

int _WGetint(_WSft *px, void *pans)
	{	/* get an integer value for _WScanf */
	wchar_t ac[ACSIZE], *p;
	char seen = 0;
	wint_t ch;
	static const wchar_t digits[] = {
		L'0', L'1', L'2', L'3', L'4', L'5',
		L'6', L'7', L'8', L'9', L'a', L'b',
		L'c', L'd', L'e', L'f', L'A', L'B',
		L'C', L'D', L'E', L'F'};
	static const wchar_t flit[] = {
		L'd', L'i', L'o', L'u', L'x', L'X',
		L'p', L'\0'};
	static const char barr[] = {10, 0, 8, 10, 16, 16, 16};
	int base =
		barr[(const wchar_t *)wcschr(&flit[0], *px->s) - flit];
	int dlen;

	px->nget = 0 < px->width ? px->width : INT_MAX;
	p = ac, ch = WGETN(px);
	if (ch == L'+' || ch == L'-')
		*p++ = ch, ch = WGETN(px);
	if (ch == L'0')
		{	/* match possible prefix */
		seen = 1;
		*p++ = ch, ch = WGETN(px);
		if ((ch == L'x' || ch == L'X')
			&& (base == 0 || base == 16))
			base = 16, *p++ = ch, ch = WGETN(px), seen = 0;
		else if (base == 0)
			base = 8;
		}
	dlen = base == 0 || base == 10 ? 10
		: base == 8 ? 8 : 16 + 6;
	for (; ch == L'0'; seen = 1)
		ch = WGETN(px);
	if (seen)
		*p++ = L'0';
	for (; ch != WEOF && wmemchr(&digits[0], ch, dlen);
		ch = WGETN(px), seen = 1)
		if (p < &ac[ACSIZE - 1])
			*p++ = ch;
	WUNGETN(px, ch);
	if (!seen)
		return (p == ac && ch == WEOF ? EOF : 0);
	*p = L'\0';
	if (px->noconv)
		;
	else if (*px->s == L'd' || *px->s == L'i')
		{	/* deliver a signed integer */
		const _Longlong lval = _WStoll(ac, 0, base);

		px->stored = 1;
		switch (px->qual)
			{	/* store in specified integer type */
		case L'b':
			if (pans != 0)
				*(signed char *)pans = (signed char)lval;
			else
				*va_arg(px->ap, signed char *) = (signed char)lval;
			break;

		case L'q':
			if (pans != 0)
				*(_Longlong *)pans = lval;
			else
				*va_arg(px->ap, _Longlong *) = lval;
			break;

		case L'j':
			if (pans != 0)
				*(intmax_t *)pans = lval;
			else
				*va_arg(px->ap, intmax_t *) = lval;
			break;

		case L't':
			if (pans != 0)
				*(ptrdiff_t *)pans = (ptrdiff_t)lval;
			else
				*va_arg(px->ap, ptrdiff_t *) = (ptrdiff_t)lval;
			break;

		case L'z':
			if (pans != 0)
				*(ptrdiff_t *)pans = (ptrdiff_t)lval;
			else
				*va_arg(px->ap, ptrdiff_t *) = (ptrdiff_t)lval;
			break;

		case L'h':
			if (pans != 0)
				*(short *)pans = (short)lval;
			else
				*va_arg(px->ap, short *) = (short)lval;
			break;

		case L'l':
			if (pans != 0)
				*(long *)pans = (long)lval;
			else
				*va_arg(px->ap, long *) = (long)lval;
			break;

		default:
			if (pans != 0)
				*(int *)pans = (int)lval;
			else
				*va_arg(px->ap, int *) = (int)lval;
			}
		}
	else
		{	/* deliver an unsigned integer */
		const _ULonglong ulval = _WStoull(ac, 0, base);

		px->stored = 1;
		if (*px->s == L'p')

 #if defined(__BORLANDC__) && !__EDG__
			if (pans != 0)
				*(void **)pans = (void *)ulval;
			else
				*va_arg(px->ap, void **) =
					(void *)ulval;	/* quiet diagnostic */

 #else /* defined(__BORLANDC__) && !__EDG__ */
			if (pans != 0)
				*(void **)pans = (void *)((char *)0 + ulval);
			else
				*va_arg(px->ap, void **) = (void *)((char *)0 + ulval);
 #endif /* defined(__BORLANDC__) && !__EDG__ */

		else
			switch (px->qual)
				{	/* store in specified integer type */
			case L'b':
				if (pans != 0)
					*(unsigned char *)pans = (unsigned char)ulval;
				else
					*va_arg(px->ap, unsigned char *) = (unsigned char)ulval;
				break;

			case L'q':
				if (pans != 0)
					*(_ULonglong *)pans = ulval;
				else
					*va_arg(px->ap, _ULonglong *) = ulval;
				break;

			case L'j':
				if (pans != 0)
					*(uintmax_t *)pans = ulval;
				else
					*va_arg(px->ap, uintmax_t *) = ulval;
				break;

			case L't':
				if (pans != 0)
					*(size_t *)pans = (size_t)ulval;
				else
					*va_arg(px->ap, size_t *) = (size_t)ulval;
				break;

			case L'z':
				if (pans != 0)
					*(size_t *)pans = (size_t)ulval;
				else
					*va_arg(px->ap, size_t *) = (size_t)ulval;
				break;

			case L'h':
				if (pans != 0)
					*(unsigned short *)pans = (unsigned short)ulval;
				else
					*va_arg(px->ap, unsigned short *) = (unsigned short)ulval;
				break;

			case L'l':
				if (pans != 0)
					*(unsigned long *)pans = (unsigned long)ulval;
				else
					*va_arg(px->ap, unsigned long *) = (unsigned long)ulval;
				break;

			default:
				if (pans != 0)
					*(unsigned int *)pans = (unsigned int)ulval;
				else
					*va_arg(px->ap, unsigned int *) = (unsigned int)ulval;
				}
		}
	return (1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwgetint.c $Rev: 153052 $");
