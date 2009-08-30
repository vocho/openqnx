/* wcstombs function */
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "xwchar.h"
_STD_BEGIN

size_t (wcstombs)(char *_Restrict s, const wchar_t *_Restrict wcs,
	size_t n)
	{	/* translate wide char string to multibyte string */
	int i;
	size_t nc;
	_Mbstinit(mbst);

	for (nc = 0; nc < n || s == 0; ++wcs)
		{	/* translate another wide character */
		if (MB_CUR_MAX <= n - nc && s != 0)
			{	/* copy directly */
			if ((i = _Wctomb(s + nc, *wcs, &mbst)) < 0)
				return ((size_t)-1);
			nc += i;
			if (0 < i && s[nc - 1] == '\0')
				return (nc - 1);
			}
		else
			{	/* copy into local buffer */
			char buf[MB_LEN_MAX];

			if ((i = _Wctomb(buf, *wcs, &mbst)) < 0)
				return ((size_t)-1);
			else if ((size_t)i <= n - nc)
				{	/* will all fit, copy and continue */
				if (s != 0)
					memcpy(s + nc, buf, i);
				nc += i;
				if (0 < i && buf[i - 1] == '\0')
					return (nc - 1);
				}
			else if (s == 0)
				nc += i;	/* won't all fit, but we're just counting */
			else
				{	/* won't all fit, copy partial and quit */
				memcpy(s + nc, buf, n - nc);
				return (n);
				}
			}
		}
	return (nc);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcstombs.c $Rev: 153052 $");
