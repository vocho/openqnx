/* _WLitob function */
#include <wchar.h>
#include "xmath.h"
#include "xwstdio.h"
_STD_BEGIN

static const wchar_t ldigs[] = {
	L'0', L'1', L'2', L'3',
	L'4', L'5', L'6', L'7',
	L'8', L'9', L'a', L'b',
	L'c', L'd', L'e', L'f'};
static const wchar_t udigs[] = {
	L'0', L'1', L'2', L'3',
	L'4', L'5', L'6', L'7',
	L'8', L'9', L'A', L'B',
	L'C', L'D', L'E', L'F'};

void _WLitob(_WPft *px, wchar_t code)
	{	/* convert unsigned long to wide text */
	wchar_t ac[24];	/* safe for 64-bit integers */
	const wchar_t *digs = code == L'X' ? udigs : ldigs;
	int base = code == L'o' ? 8 :
		code != L'x' && code != L'X' ? 10 : 16;
	int i = sizeof (ac) / sizeof (wchar_t);
	_ULonglong ulval = px->v.li;

	if ((code == L'd' || code == L'i') && px->v.li < 0)
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
		&& (sizeof (ac) / sizeof (wchar_t) <= i || ac[i] != L'0'))
		ac[--i] = L'0';
	px->n1 = sizeof (ac) / sizeof (wchar_t) - i;
	wmemcpy(px->s, &ac[i], px->n1);

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

__SRCVERSION("xwlitob.c $Rev: 153052 $");
