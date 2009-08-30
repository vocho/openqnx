/* wcsftime function */
#include "xtime.h"
#include "xwchar.h"
_STD_BEGIN

size_t (wcsftime)(wchar_t *_Restrict buf, size_t bufsize,
	const wchar_t *_Restrict fmt, const struct tm *_Restrict t)
	{	/* format wide time information */
	int ch;
	const wchar_t *ibuf = buf;

	while (0 < bufsize && fmt[0] != L'\0')
		if (fmt[0] != L'%' || (ch = wctob(fmt[1])) <= 0)
			*buf++ = *fmt++, --bufsize;
		else
			{	/* process a conversion specifier */
			char nfmt[3];
			int n = 2;

			nfmt[0] = '%', nfmt[1] = (char)ch, fmt += 2;
			if ((ch == 'E' || ch == 'O') && fmt[0] != L'\0')
				nfmt[n++] = (char)wctob(*fmt++);
			if ((n = _Wcsftime(buf, bufsize, nfmt, n, t)) < 0)
				return (0);
			buf += n, bufsize -= n;
			}
	if (bufsize == 0)
		return (0);
	*buf = L'\0';
	return (buf - (wchar_t *)ibuf);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcsftime.c $Rev: 153052 $");
