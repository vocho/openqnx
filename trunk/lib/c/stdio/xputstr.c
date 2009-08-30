/* _Putstr function */
#include <string.h>
#include <stdlib.h>
#include "xstdio.h"
#include "xwchar.h"
_STD_BEGIN

#define BUF_SIZE	64

int _Putstr(_Pft *px, const wchar_t *pwc)
	{	/* convert wchar_t string to text */
	char ac[BUF_SIZE < MB_LEN_MAX ? MB_LEN_MAX : BUF_SIZE];
	char buf[MB_LEN_MAX], *pac;
	int m = px->prec < 0 ? INT_MAX : px->prec;
	int n, stat;
	size_t acsize = px->width + MB_CUR_MAX;
	_Mbstinit(mbst);

	if (px->flags & _FMI || acsize <= sizeof (ac))
		pac = ac, acsize = sizeof (ac);
	else if ((pac = (char *)malloc(acsize)) == 0)
		return (EOF);
	for (stat = 0; 0 < m; ++pwc, m -= n)
		{	/* convert a wide character */
		if ((n = _Wctomb(buf, *pwc, &mbst)) < 0
			|| *pwc == 0 && --n < 0)
			{	/* stop on bad conversion */
			stat = EOF;
			break;
			}
		else if (m < n)
			break;	/* precision exhausted */
		if (acsize < (size_t)(px->n0 + n))
			{	/* drain buffer */
			px->width = 0;
			if ((stat = _Puttxt(px, pac)) < 0)
				break;
			px->n0 = 0;
			}
		memcpy(&pac[px->n0], buf, n);
		px->n0 += n;
		if (*pwc == L'\0')
			break;
		}
	if (stat == 0)
		stat = _Puttxt(px, pac);
	if (stat == 0)
		px->n0 = px->width = 0;
	if (pac != ac)
		free(pac);
	return (stat);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xputstr.c $Rev: 153052 $");
