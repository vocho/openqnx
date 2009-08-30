/* _Iswctype function */
 #include "xmtloc.h"
_STD_BEGIN

int _Iswctype(wint_t wc, wctype_t off)
	{	/* classify wide character */
	_PWctab pwctype = *_TLS_DATA_PTR(_Wctype);
	const wchar_t *p =
		(const wchar_t *)pwctype[0]._Name + pwctype[off]._Off;
	const wchar_t *pe =
		(const wchar_t *)pwctype[0]._Name + pwctype[off + 1]._Off;

	for (; p < pe; p += 3)
		if (p[0] <= wc && wc <= p[1])
			return ((int)p[2]);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwctype.c $Rev: 153052 $");
