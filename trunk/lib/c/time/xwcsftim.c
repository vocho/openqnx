/* _Wcsftime function */
#include <stdlib.h>
#include <string.h>
#include "xmtloc.h"
#include "xtime.h"
#include "xwchar.h"
_STD_BEGIN

#define NSTACK	3	/* depth of format nesting stack */

size_t _Wcsftime(wchar_t *buf, size_t bufsize,
	const char *fmt, size_t len, const struct tm *t)
	{	/* format and widen time information */
	const char *fmtsav[NSTACK] = { NULL };
	size_t lensav[NSTACK] = { 0 };
	size_t nstack = 0;
	wchar_t *ibuf = buf;
	_Mbstinit(mbst);

	while (0 < len || 0 < nstack)
		{	/* parse format string */
		int n;
		wchar_t wc = L'\0';

		if (len == 0)
			fmt = fmtsav[--nstack], len = lensav[nstack];
		if ((n = _Mbtowc(&wc, fmt, len, &mbst)) <= 0)
			n = *fmt == '\0' ? 0 : 1;	/* bad parse, eat one char */
		fmt += n, len -= n;
		if (wc == L'\0')
			;	/* discard any trailer */
		else if (bufsize == 0)
			return (0);	/* not enough room */
		else if (wc != L'%' || len == 0)
			*buf++ = wc, --bufsize;
		else
			{	/* do the conversion */
			char ac[20];
			char qual = (char)(*fmt == 'E' || *fmt == 'O' ? *fmt++ : '\0');
			int m;
			const char *p;

			p = _Gentime(t, _TLS_DATA_PTR(_Times), qual, *fmt, &m, ac);
			if (qual != '\0')
				--len;
			++fmt, --len;
			if (0 < m)
				{	/* parse conversion string */
				_Mbstinit(mbst2);

				for (; 0 < m; p += n, m -= n)
					if ((n = _Mbtowc(&wc, p, m, &mbst2)) <= 0)
						break;
					else if (bufsize == 0)
						return (0);
					else
						*buf++ = wc, --bufsize;
				}
			else if (len == 0 || NSTACK <= nstack)
				fmt = p, len = -m;	/* tail recursion or stack full */
			else
				{	/* add leftover format to stack */
				fmtsav[nstack] = fmt, fmt = p;
				lensav[nstack++] = len, len = -m;
				}
			}
		}
	return (buf - ibuf);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwcsftim.c $Rev: 153052 $");
