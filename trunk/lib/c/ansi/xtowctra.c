/* _Towctrans function */
#include <wctype.h>
#include "xmtloc.h"
#include "xwctype.h"
_STD_BEGIN

wint_t _Towctrans(wint_t wc, wctrans_t off)
	{	/* translate wide character */
	_PWctab pwctrans = *_TLS_DATA_PTR(_Wctrans);
	const wchar_t *p =
		(const wchar_t *)pwctrans[0]._Name + pwctrans[off]._Off;
	const wchar_t *pe =
		(const wchar_t *)pwctrans[0]._Name + pwctrans[off + 1]._Off;

	for (; p < pe; p += 3)
		if (p[0] <= wc && wc <= p[1])
			return ((wint_t)(wc + (wint_t)p[2]));
	return (wc);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xtowctra.c $Rev: 153052 $");
