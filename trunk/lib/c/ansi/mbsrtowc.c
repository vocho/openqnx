/* mbsrtowcs function */
#include <limits.h>
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(static, mbstate_t, mbst, {0});

size_t (mbsrtowcs)(wchar_t *_Restrict wcs, const char **_Restrict ps,
	size_t n, mbstate_t *_Restrict pst)
	{	/* translate multibyte string to wide, restartably */
	const char *s = *ps;
	int i;
	size_t nwc = 0;

	if (pst == 0)
		pst = _TLS_DATA_PTR(mbst);
	if (wcs == 0)
		for (; ; ++nwc, s += i)
			{	/* translate but don't store */
			wchar_t wc;

			if ((i = _Mbtowc(&wc, s, INT_MAX, pst)) < 0)
				return ((size_t)-1);
			else if (i == 0 && wc == L'\0')
				return (nwc);
			}
	for (; 0 < n; ++nwc, s += i, ++wcs, --n)
		{	/* translate and store */
		if ((i = _Mbtowc(wcs, s, INT_MAX, pst)) < 0)
			{	/* encountered invalid sequence */
			nwc = (size_t)-1;
			break;
			}
		else if (i == 0 && *wcs == L'\0')
			{	/* encountered terminating null */
			s = 0;
			break;
			}
		}
	*ps = s;
	return (nwc);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mbsrtowc.c $Rev: 153052 $");
