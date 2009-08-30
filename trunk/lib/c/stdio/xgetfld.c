/* _Getfld function */
#include <ctype.h>
#include <string.h>
#include "xstdio.h"

 #if _HAS_FIXED_POINT
 #include "fixed.h"		/* internal fixed-point header */

int _Fixed_get(_Sft *, fxmaskf);

int _Getcvec(_Sft *);	/* vc */
int _Getfvec(_Sft *);	/* ve, vf, etc. */
int _Geticvec(_Sft *);	/* vd, vi, vo, vu, vx, vX */
int _Getihvec(_Sft *);	/* hvd, vhd, etc. */
int _Getilvec(_Sft *);	/* lvd, dlv, etc. */
 #endif /* _HAS_FIXED_POINT */

_STD_BEGIN

int _Getfld(_Sft *px)
	{	/* convert a field */
	px->stored = 0;
	switch (*px->s)
		{	/* switch on conversion specifier */
	case 'c':	/* convert an array of char */

 #if _HAS_FIXED_POINT
		if (px->qual == 'v')
			return (_Getcvec(px));
 #endif /* _HAS_FIXED_POINT */

		return (_Getstr(px, 0));

	case 'd': case 'i': case 'o':
	case 'u': case 'x': case 'X':

 #if _HAS_FIXED_POINT
		if (px->qual == 'v')
			return (_Geticvec(px));	/* v */
		else if (px->qual == 'w')
			return (_Getihvec(px));	/* hv or vh */
		else if (px->qual == 'W')
			return (_Getilvec(px));	/* hl or lh */
 #endif /* _HAS_FIXED_POINT */

	case 'p':	/* convert a pointer */
		return (_Getint(px, 0));	/* convert an integer */

	case 'e': case 'E':
	case 'g': case 'G':
	case 'f': case 'F':
	case 'a': case 'A':

 #if _HAS_FIXED_POINT
		if (px->qual == 'v')
			return (_Getfvec(px));
 #endif /* _HAS_FIXED_POINT */

		return (_Getfloat(px, 0));	/* convert a floating */

 #if _HAS_FIXED_POINT
	case 'k':	/* convert a fixed signed accumulator */
		return (_Fixed_get(px, FX_ACCUM));

	case 'K':	/* convert a fixed unsigned accumulator */
		return (_Fixed_get(px, FX_ACCUM | FX_UNSIGNED));

	case 'r':	/* convert a fixed signed fraction */
		return (_Fixed_get(px, 0));

	case 'R':	/* convert a fixed unsigned fraction */
		return (_Fixed_get(px, FX_UNSIGNED));
 #endif /* _HAS_FIXED_POINT */

	case 'n':	/* return input count */
		if (!px->noconv)
			switch (px->qual)
				{	/* store in specified integer type */
			case 'b':
				*va_arg(px->ap, signed char *) = (signed char)px->nchar;
				break;

			case 'q':
				*va_arg(px->ap, _Longlong *) = px->nchar;
				break;

			case 'j':
				*va_arg(px->ap, intmax_t *) = px->nchar;
				break;

			case 't':
				*va_arg(px->ap, ptrdiff_t *) = px->nchar;
				break;

			case 'z':
				*va_arg(px->ap, size_t *) = px->nchar;
				break;

			case 'h':
				*va_arg(px->ap, short *) = (short)px->nchar;
				break;

			case 'l':
				*va_arg(px->ap, long *) = px->nchar;
				break;

			default:
				*va_arg(px->ap, int *) = px->nchar;
				}
		return (1);

	case 'S':	/* convert a wide string -- nonstandard */
		px->qual = 'l';	/* fall through */

	case 's':	/* convert a string */
		return (_Getstr(px, 1));

	case '%':
		 {	/* match a '%' */
		int ch;

		if ((ch = GET(px)) == '%')
			return (1);
		UNGETN(px, ch);
		return (ch == EOF ? EOF : 0);
		 }

	case '[':	/* convert a scan set */
		return (_Getstr(px, -1));

	default:	/* undefined specifier, quit */
		return (0);
		}
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xgetfld.c $Rev: 153052 $");
