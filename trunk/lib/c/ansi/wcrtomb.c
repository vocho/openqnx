/* wcrtomb function */
#include <limits.h>
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(static, mbstate_t, mbst, {0});

size_t (wcrtomb)(char *_Restrict s, wchar_t wchar,
	mbstate_t *_Restrict pst)
	{	/* translate wchar_t to multibyte, restartably */
	char buf[MB_LEN_MAX];

	if (pst == 0)
		pst = _TLS_DATA_PTR(mbst);
	return (s != 0 ? _Wctomb(s, wchar, pst) : _Wctomb(buf, L'0', pst));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcrtomb.c $Rev: 153052 $");
