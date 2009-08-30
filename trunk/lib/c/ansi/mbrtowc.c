/* mbrtowc function */
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(static, mbstate_t, mbst, {0});

size_t (mbrtowc)(wchar_t *_Restrict pwc, const char *_Restrict s,
	size_t n, mbstate_t *_Restrict pst)
	{	/*	translate multibyte to wchar_t, restartably */
	if (pst == 0)
		pst = _TLS_DATA_PTR(mbst);

	return (s != 0 ? _Mbtowc(pwc, s, n, pst) : _Mbtowc(0, "", n, pst));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mbrtowc.c $Rev: 153052 $");
