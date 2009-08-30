/* mbtowc function */
#include <stdlib.h>
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(static, mbstate_t, mbst, {0});

int (mbtowc)(wchar_t *_Restrict pwc, const char *_Restrict s, size_t n)
	{	/*	determine next multibyte code */
	int i = _Mbtowc(pwc, s,
		n <= MB_CUR_MAX ? n : MB_CUR_MAX, _TLS_DATA_PTR(mbst));

	return (i < 0 ? -1 : i);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mbtowc.c $Rev: 153052 $");
