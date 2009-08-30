/* _WGetfld function */
#include "xwstdio.h"

 #if _HAS_FIXED_POINT
 #include "fixed.h"		/* internal fixed-point header */

int _Fixed_wget(_WSft *, fxmaskf);

int _WGetcvec(_WSft *);		/* vc */
int _WGetfvec(_WSft *);		/* ve, vf, etc. */
int _WGeticvec(_WSft *);	/* vd, vi, vo, vu, vx, vX */
int _WGetihvec(_WSft *);	/* hvd, vhd, etc. */
int _WGetilvec(_WSft *);	/* lvd, dlv, etc. */
 #endif /* _HAS_FIXED_POINT */

_STD_BEGIN

int _WGetfld(_WSft *px)
	{	/* convert a wide field */
	px->stored = 0;
	switch (*px->s)
		{	/* switch on conversion specifier */
	case L'c':	/* convert an array of chars */

 #if _HAS_FIXED_POINT
		if (px->qual == L'v')
			return (_WGetcvec(px));
 #endif /* _HAS_FIXED_POINT */

		return (_WGetstr(px, 0));

	case L'd': case L'i': case L'o':
	case L'u': case L'x': case L'X':

 #if _HAS_FIXED_POINT
		if (px->qual == L'v')
			return (_WGeticvec(px));	/* v */
		else if (px->qual == L'w')
			return (_WGetihvec(px));	/* hv or vh */
		else if (px->qual == L'W')
			return (_WGetilvec(px));	/* hl or lh */
 #endif /* _HAS_FIXED_POINT */

	case L'p':	/* convert a pointer */
		return (_WGetint(px, 0));	/* convert an integer */

	case L'e': case L'E':
	case L'g': case L'G':
	case L'f': case L'F':
	case L'a': case L'A':

 #if _HAS_FIXED_POINT
		if (px->qual == L'v')
			return (_WGetfvec(px));
 #endif /* _HAS_FIXED_POINT */

		return (_WGetfloat(px, 0));	/* convert a floating */

 #if _HAS_FIXED_POINT
	case 'k':	/* convert a fixed signed accumulator */
		return (_Fixed_wget(px, FX_ACCUM));

	case 'K':	/* convert a fixed unsigned accumulator */
		return (_Fixed_wget(px, FX_ACCUM | FX_UNSIGNED));

	case 'r':	/* convert a fixed signed fraction */
		return (_Fixed_wget(px, 0));

	case 'R':	/* convert a fixed unsigned fraction */
		return (_Fixed_wget(px, FX_UNSIGNED));
 #endif /* _HAS_FIXED_POINT */

	case L'n':	/* return input count */
		if (!px->noconv)
			switch (px->qual)
				{	/* store in specified integer type */
			case L'b':
				*va_arg(px->ap, signed char *) = (signed char)px->nchar;
				break;

			case L'q':
				*va_arg(px->ap, _Longlong *) = px->nchar;
				break;

			case L'j':
				*va_arg(px->ap, intmax_t *) = px->nchar;
				break;

			case L't':
				*va_arg(px->ap, ptrdiff_t *) = px->nchar;
				break;

			case L'z':
				*va_arg(px->ap, size_t *) = px->nchar;
				break;

			case L'h':
				*va_arg(px->ap, short *) = (short)px->nchar;
				break;

			case L'l':
				*va_arg(px->ap, long *) = px->nchar;
				break;

			default:
				*va_arg(px->ap, int *) = px->nchar;
				}
		return (1);

	case L's':	/* convert a multibyte string */
		return (_WGetstr(px, 1));

	case L'%':	/* match a '%' */
		 {	/* match a '%' */
		wint_t ch;

		if ((ch = WGET(px)) == L'%')
			return (1);
		WUNGETN(px, ch);
		return (ch == WEOF ? EOF : 0);
		 }

	case L'[':	/* convert a scan set */
		return (_WGetstr(px, -1));

	default:	/* undefined specifier, quit */
		return (0);
		}
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwgetfld.c $Rev: 153052 $");
