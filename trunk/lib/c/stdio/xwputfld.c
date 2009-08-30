/* _WPutfld function */
#include "xmath.h"
#include "xwstdio.h"

 #if _HAS_FIXED_POINT
 #include "fixed.h"		/* internal fixed-point header */

 #define GET_SFIXED(ty)	\
	(sizeof (ty) <= sizeof (int) ? va_arg(*pap, int) \
	: sizeof(ty) <= sizeof (long) ? va_arg(*pap, long) \
	: va_arg(*pap, _Longlong))
 #define GET_UFIXED(ty)	\
	(sizeof (ty) <= sizeof (unsigned int) ? va_arg(*pap, unsigned int) \
	: sizeof(ty) <= sizeof (unsigned long) ? va_arg(*pap, unsigned long) \
	: va_arg(*pap, _ULonglong))

void _Fixed_wput(_WPft *, fxmaskf);

int _WPutcvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* vc */
int _WPutfvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* ve, vf, etc. */
int _WPutscvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* vd, vi */
int _WPutshvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* hvd, vhd, etc. */
int _WPutslvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* lvd, dlv, etc. */
int _WPutucvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* vo, vu, etc. */
int _WPutuhvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* hvo, vho, etc. */
int _WPutulvec(_WPft *, va_list *, wchar_t, wchar_t *);	/* hlo, lho, etc. */
 #endif /* _HAS_FIXED_POINT */

_STD_BEGIN

int _WPutfld(_WPft *px, va_list *pap, wchar_t code,
	wchar_t *ac)
	{	/* convert a field for _WPrintf */
	switch (code)
		 {	/* switch on conversion specifier */
	case L'c':	/* convert a single character */

 #if _HAS_FIXED_POINT
		if (px->qual == L'v')
			return (_WPutcvec(px, pap, code, ac));
 #endif /* _HAS_FIXED_POINT */

		if (px->qual == L'l')

 #if WCHAR_MAX <= INT_MAX
			ac[px->n0++] = va_arg(*pap, int);

 #else /* WCHAR_MAX <= INT_MAX */
			ac[px->n0++] = va_arg(*pap, wint_t);
 #endif /* WCHAR_MAX <= INT_MAX */

		else
			{	/* check conversion before storing */
			wint_t wc = _Btowc(va_arg(*pap, int));

			if (wc == WEOF)
				return (EOF);
			ac[px->n0++] = wc;
			}
		break;

	case L'd': case L'i':	/* convert a signed decimal */

 #if _HAS_FIXED_POINT
		if (px->qual == L'v')
			return (_WPutscvec(px, pap, code, ac));	/* v */
		else if (px->qual == L'w')
			return (_WPutshvec(px, pap, code, ac));	/* hv or vh */
		else if (px->qual == L'W')
			return (_WPutslvec(px, pap, code, ac));	/* hl or lh */
 #endif /* _HAS_FIXED_POINT */

		px->v.li = px->qual == L'l' ? va_arg(*pap, long)
			: px->qual == L'q' ? va_arg(*pap, _Longlong)
			: px->qual == L'j' ? va_arg(*pap, intmax_t)
			: va_arg(*pap, int);
		if (px->qual == L'h')
			px->v.li = (short)px->v.li;
		else if (px->qual == L'b')
			px->v.li = (signed char)px->v.li;
		else if (px->qual == L't' || px->qual == L'z')
			px->v.li = (ptrdiff_t)px->v.li;
		if (px->v.li < 0)	/* negate safely in _WLitob */
			ac[px->n0++] = L'-';
		else if (px->flags & _FPL)
			ac[px->n0++] = L'+';
		else if (px->flags & _FSP)
			ac[px->n0++] = L' ';
		px->s = &ac[px->n0];
		_WLitob(px, code);
		break;

	case L'o': case L'u':
	case L'x': case L'X':	/* convert unsigned */

 #if _HAS_FIXED_POINT
		if (px->qual == L'v')
			return (_WPutucvec(px, pap, code, ac));	/* v */
		else if (px->qual == L'w')
			return (_WPutuhvec(px, pap, code, ac));	/* hv or vh */
		else if (px->qual == L'W')
			return (_WPutulvec(px, pap, code, ac));	/* hl or lh */
 #endif /* _HAS_FIXED_POINT */

		px->v.li = px->qual == L'l' ? va_arg(*pap, unsigned long)
			: px->qual == L'q' ? va_arg(*pap, _ULonglong)
			: px->qual == L'j' ? va_arg(*pap, uintmax_t)
			: va_arg(*pap, unsigned int);
		if (px->qual == L'h')
			px->v.li = (unsigned short)px->v.li;
		else if (px->qual == L'b')
			px->v.li = (unsigned char)px->v.li;
		else if (px->qual == L't' || px->qual == L'z')
			px->v.li = (size_t)px->v.li;
		if (px->flags & _FNO && px->v.li != 0
			&& (code == L'x' || code == L'X'))
			ac[px->n0++] = L'0', ac[px->n0++] = code;
		px->s = &ac[px->n0];
		_WLitob(px, code);
		break;

	case L'e': case L'E':	/* convert floating */
	case L'g': case L'G':
	case L'f': case L'F':
	case L'a': case L'A':

 #if _HAS_FIXED_POINT
		if (px->qual == L'v')
			return (_WPutfvec(px, pap, code, ac));
 #endif /* _HAS_FIXED_POINT */

		px->v.ld = px->qual == L'L'
			? va_arg(*pap, long double) : va_arg(*pap, double);
		if (LSIGN(px->v.ld))
			ac[px->n0++] = L'-';
		else if (px->flags & _FPL)
			ac[px->n0++] = L'+';
		else if (px->flags & _FSP)
			ac[px->n0++] = L' ';
		px->s = &ac[px->n0];
		_WLdtob(px, code);
		break;

 #if _HAS_FIXED_POINT
	case L'k':	/* convert fixed signed accumulator */
		if (px->qual == L'l')
			px->v.li = GET_SFIXED(_Fixed_lk);
		else if (px->qual == L'h')
			px->v.li = GET_SFIXED(_Fixed_hk);
		else
			px->v.li = GET_SFIXED(_Fixed_k);

		px->s = &ac[px->n0];
		_Fixed_wput(px, FX_ACCUM);
		break;

	case L'K':	/* convert fixed unsigned accumulator */
		if (px->qual == L'l')
			px->v.li = GET_UFIXED(_Fixed_ulk);
		else if (px->qual == L'h')
			px->v.li = GET_UFIXED(_Fixed_uhk);
		else
			px->v.li = GET_UFIXED(_Fixed_uk);

		px->s = &ac[px->n0];
		_Fixed_wput(px, FX_ACCUM | FX_UNSIGNED);
		break;

	case L'r':	/* convert fixed signed fraction */
		if (px->qual == L'l')
			px->v.li = GET_SFIXED(_Fixed_lr);
		else if (px->qual == L'h')
			px->v.li = GET_SFIXED(_Fixed_hr);
		else
			px->v.li = GET_SFIXED(_Fixed_r);

		px->s = &ac[px->n0];
		_Fixed_wput(px, 0);
		break;

	case L'R':	/* convert fixed unsigned fraction */
		if (px->qual == L'l')
			px->v.li = GET_UFIXED(_Fixed_ulr);
		else if (px->qual == L'h')
			px->v.li = GET_UFIXED(_Fixed_uhr);
		else
			px->v.li = GET_UFIXED(_Fixed_ur);

		px->s = &ac[px->n0];
		_Fixed_wput(px, FX_UNSIGNED);
		break;
 #endif /* _HAS_FIXED_POINT */

	case L'n':	/* return output count */
		switch (px->qual)
			{	/* store in specified integer type */
		case L'b':
			*va_arg(*pap, signed char *) = px->nchar;
			break;

		case L'q':
			*va_arg(*pap, _Longlong *) = px->nchar;
			break;

		case L'j':
			*va_arg(*pap, intmax_t *) = px->nchar;
			break;

		case L't':
			*va_arg(*pap, ptrdiff_t *) = px->nchar;
			break;

		case L'z':
			*va_arg(*pap, size_t *) = px->nchar;
			break;

		case L'h':
			*va_arg(*pap, short *) = px->nchar;
			break;

		case L'l':
			*va_arg(*pap, long *) = px->nchar;
			break;

		default:
			*va_arg(*pap, int *) = px->nchar;
			}
		break;

		case L'p':
		 {	/* convert a pointer, hex long version */
		static const size_t vpsize = sizeof (void *);

		px->v.li = (_Longlong)((char *)va_arg(*pap, void *) - (char *)0);
		if (vpsize == sizeof (unsigned long))
			px->v.li &= ULONG_MAX;
		px->s = &ac[px->n0];
		_WLitob(px, L'x');
		 }
		break;

	case L's':	/* convert a string */
		if (px->qual == L'l')
			{	/* determine length safely */
			wchar_t *s1;

			px->s = va_arg(*pap, wchar_t *);
			px->n1 = px->prec < 0 ? wcslen(px->s)
				: (s1 = (wchar_t *)wmemchr(px->s, L'\0',
					px->prec)) != 0 ? s1 - px->s : px->prec;
			 }
		else if (_WPutstr(px, va_arg(*pap, const char *))
			== EOF)
			return (EOF);
		break;

	case L'%':	/* put a '%' */
		ac[px->n0++] = L'%';
		break;

	default:	/* undefined specifier, print it out */
		ac[px->n0++] = L'%';
		if (code != '\0')
			ac[px->n0++] = code;
			}
	return (0);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwputfld.c $Rev: 153052 $");
