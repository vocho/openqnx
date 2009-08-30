/* wcsxfrm function */
 #include "xwcsxfrm.h"
 #include "xmtloc.h"
_STD_BEGIN

size_t _Wcsxfrmx(wchar_t *s1, const wchar_t *s2, size_t n,
	_Statab *pcostate)
	{	/* transform s2[] to s1[] using given locale-dependent rule */
	size_t nx = 0;
	const wchar_t *s = s2;
	_Mbstinit(mbst);

	while (nx < n)
		{	/* translate and deliver */
		size_t i = _CWcsxfrm(s1, &s, n - nx, &mbst, pcostate);

		s1 += i, nx += i;
		if (0 < i && s1[-1] == L'\0')
			return (nx - 1);
		else if (nx < n && *s == L'\0')
			s = s2;	/* rescan */
		}
	for (; ; )
		{	/* translate and count */
		wchar_t buf[32];
		size_t i = _CWcsxfrm(buf, &s,
			sizeof (buf) / sizeof (wchar_t), &mbst, pcostate);

		nx += i;
		if (0 < i && buf[i - 1] == L'\0')
			return (nx - 1);
		else if (i < sizeof (buf) / sizeof (wchar_t) && *s == L'\0')
			s = s2;	/* rescan */
		}
	}

size_t (wcsxfrm)(wchar_t *_Restrict s1, const wchar_t *_Restrict s2,
	size_t n)
	{	/* transform s2[] to s1[] using global locale-dependent rule */
	return (_Wcsxfrmx(s1, s2, n, _TLS_DATA_PTR(_WCostate)));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcsxfrm.c $Rev: 153052 $");
