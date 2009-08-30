/* _Getint function */
#include <stdlib.h>
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

#define ACSIZE	32	/* holds only prefix, m.s. digits */

int _Getint(_Sft *px, void *pans)
	{	/* get an integer value for _Scanf */
	char ac[ACSIZE], *p, seen;
	int base, ch, dlen;
	static const char digits[] = "0123456789abcdefABCDEF";
	static const char flit[] = "diouxXp";
	static const char barr[] = {10, 0, 8, 10, 16, 16, 16};

	px->nget = 0 < px->width ? px->width : INT_MAX;
	p = ac, ch = GETN(px);
	if (ch == '+' || ch == '-')
		*p++ = (char)ch, ch = GETN(px);
	seen = 0;
	base = barr[(const char *)strchr(&flit[0], *px->s) - flit];
	if (ch == '0')
		{	/* match possible prefix and strip it */
		seen = 1;
		ch = GETN(px);
		if ((ch == 'x' || ch == 'X')
			&& (base == 0 || base == 16))
			base = 16, ch = GETN(px), seen = 0;
		else if (base == 0)
			base = 8;
		}
	dlen = base == 0 || base == 10 ? 10
		: base == 8 ? 8 : 16 + 6;

	for (; ch == '0'; seen = 1)
		ch = GETN(px);
	if (seen)
		*p++ = '0';
	for (; ch != EOF && memchr(&digits[0], ch, dlen);
		ch = GETN(px), seen = 1)
		if (p < &ac[ACSIZE - 1])
			*p++ = (char)ch;
	UNGETN(px, ch);
	if (!seen)
		return (p == ac && ch == EOF ? EOF : 0);
	*p = '\0';
	if (px->noconv)
		;
	else if (*px->s == 'd' || *px->s == 'i')
		{	/* deliver a signed integer */
		const _Longlong lval = _Stoll(ac, 0, base);

		px->stored = 1;
		switch (px->qual)
			{	/* store in specified integer type */
		case 'b':
			if (pans != 0)
				*(signed char *)pans = (signed char)lval;
			else
				*va_arg(px->ap, signed char *) = (signed char)lval;
			break;

		case 'q':
			if (pans != 0)
				*(_Longlong *)pans = lval;
			else
				*va_arg(px->ap, _Longlong *) = lval;
			break;

		case 'j':
			if (pans != 0)
				*(intmax_t *)pans = lval;
			else
				*va_arg(px->ap, intmax_t *) = lval;
			break;

		case 't':
			if (pans != 0)
				*(ptrdiff_t *)pans = (ptrdiff_t)lval;
			else
				*va_arg(px->ap, ptrdiff_t *) = (ptrdiff_t)lval;
			break;

		case 'z':
			if (pans != 0)
				*(ptrdiff_t *)pans = (ptrdiff_t)lval;
			else
				*va_arg(px->ap, ptrdiff_t *) = (ptrdiff_t)lval;
			break;

		case 'h':
			if (pans != 0)
				*(short *)pans = (short)lval;
			else
				*va_arg(px->ap, short *) = (short)lval;
			break;

		case 'l':
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
		const _ULonglong ulval = _Stoull(ac, 0, base);

		px->stored = 1;
		if (*px->s == 'p')
 

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
			case 'b':
				if (pans != 0)
					*(unsigned char *)pans = (unsigned char)ulval;
				else
					*va_arg(px->ap, unsigned char *) = (unsigned char)ulval;
				break;

			case 'q':
				if (pans != 0)
					*(_ULonglong *)pans = ulval;
				else
					*va_arg(px->ap, _ULonglong *) = ulval;
				break;

			case 'j':
				if (pans != 0)
					*(uintmax_t *)pans = ulval;
				else
					*va_arg(px->ap, uintmax_t *) = ulval;
				break;

			case 't':
				if (pans != 0)
					*(size_t *)pans = (size_t)ulval;
				else
					*va_arg(px->ap, size_t *) = (size_t)ulval;
				break;

			case 'z':
				if (pans != 0)
					*(size_t *)pans = (size_t)ulval;
				else
					*va_arg(px->ap, size_t *) = (size_t)ulval;
				break;

			case 'h':
				if (pans != 0)
					*(unsigned short *)pans = (unsigned short)ulval;
				else
					*va_arg(px->ap, unsigned short *) = (unsigned short)ulval;
				break;

			case 'l':
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

__SRCVERSION("xgetint.c $Rev: 153052 $");
