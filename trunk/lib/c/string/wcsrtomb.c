/* wcsrtombs function */
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(static, mbstate_t, mbst, {0});

size_t (wcsrtombs)(char *_Restrict s, const wchar_t **_Restrict pwcs,
	size_t n, mbstate_t *_Restrict pst)
	{	/* translate wide char string to multibyte string */
	char buf[MB_LEN_MAX];
	int i;
	size_t nc = 0;
	const wchar_t *wcs = *pwcs;

	if (pst == 0)
		pst = _TLS_DATA_PTR(mbst);
	if (s == 0)
		for (; ; nc += i, ++wcs)
			{	/* translate but don't store */
			if ((i = _Wctomb(buf, *wcs, pst)) < 0)
				return ((size_t)-1);
			else if (0 < i && buf[i - 1] == '\0')
				return (nc + i - 1);
			}
	for (; 0 < n; nc += i, ++wcs, s += i, n -= i)
		{	/* translate and store */
		char *t;
		mbstate_t mbstsave;

		if (n < MB_CUR_MAX)
			t = buf, mbstsave = *pst;
		else
			t = s;
		if ((i = _Wctomb(t, *wcs, pst)) < 0)
			{	/* encountered invalid sequence */
			nc = (size_t)-1;
			break;
			}
		if (s == t)
			;
		else if (n < (size_t)i)
			{	/* won't all fit */
			*pst = mbstsave;
			break;
			}
		else
			memcpy(s, buf, i);
		if (0 < i && s[i - 1] == '\0')
			{	/* encountered terminating null */
			*pwcs = 0;
			return (nc + i - 1);
			}
		}
	*pwcs = wcs;
	return (nc);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcsrtomb.c $Rev: 153052 $");
