/* _Litob function */
#include <string.h>
#include "xmath.h"
#include "xstdio.h"
_STD_BEGIN

static const char ldigs[] = "0123456789abcdef";
static const char udigs[] = "0123456789ABCDEF";

void _Litob(_Pft *px, char code)
	{	/* convert unsigned long to text */
	char ac[24];	/* safe for 64-bit integers */
	const char *digs = code == 'X' ? udigs : ldigs;
	int base = code == 'o' ? 8 :
		code != 'x' && code != 'X' ? 10 : 16;
	int i = sizeof (ac);
	_ULonglong ulval = px->v.li;

	if ((code == 'd' || code == 'i') && px->v.li < 0)
		ulval = 0 - ulval;	/* safe against overflow */

	if (ulval != 0 || px->prec != 0)
		ac[--i] = digs[(size_t)(ulval % (unsigned)base)];
	px->v.li = (_Longlong)(ulval / (_ULonglong)(_Longlong)base);

	while (0 < px->v.li && 0 < i)
		{ 	/* convert digits */
		_Longlong quot = px->v.li / (_Longlong)base;

		ac[--i] = digs[(int)(px->v.li - quot * (_Longlong)base)];
		px->v.li = quot;
		}

	if (base == 8 && px->flags & _FNO
		&& (sizeof (ac) <= i || ac[i] != '0'))
		ac[--i] = '0';
	px->n1 = sizeof (ac) - i;
	memcpy(px->s, &ac[i], px->n1);

	if (px->n1 < px->prec)
		px->nz0 = px->prec - px->n1, px->flags &= ~_FZE;
	else if (px->prec < 0 && (px->flags & (_FMI|_FZE)) == _FZE
		&& 0 < (i = ((px->width - px->n0) - px->nz0) - px->n1))
		px->nz0 = i;
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xlitob.c $Rev: 153052 $");
