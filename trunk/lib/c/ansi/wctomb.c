/* wctomb function */
#include <stdlib.h>
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(static, mbstate_t, mbst, {0});

		/* static data */
int (wctomb)(char *s, wchar_t wchar)
	{	/* translate wide character to multibyte string */
	return (_Wctomb(s, wchar, _TLS_DATA_PTR(mbst)));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wctomb.c $Rev: 153052 $");
