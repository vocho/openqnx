/* _Btowc function */
#include <stdio.h>
#include "xwchar.h"
_STD_BEGIN

wint_t _Btowc(int c)
	{	/* internal function to convert single byte */
	if (c == EOF)
		return (WEOF);
	else
		{	/* convert as one-byte string */
		char ch;
		wchar_t wc;
		_Mbstinit(mbst);

		ch = (char)c;
		return ((wint_t)(_Mbtowc(&wc, &ch, 1, &mbst) < 0 ? WEOF : wc));
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xbtowc.c $Rev: 153052 $");
