/* _WPutstr function */
#include <stdlib.h>
#include "xwstdio.h"
#include "xwchar.h"
_STD_BEGIN

#define BUF_SIZE	64

int _WPutstr(_WPft *px, const char *pc)
	{	/* convert byte string to wchar_ts */
	int m = px->prec < 0 ? INT_MAX : px->prec;
	int n, stat;
	size_t acsize = px->width;
	wchar_t ac[BUF_SIZE], *pac;
	_Mbstinit(mbst);

	if (px->flags & _FMI ||
		acsize <= sizeof (ac) / sizeof (wchar_t))
		pac = ac, acsize = sizeof (ac) / sizeof (wchar_t);
	else if ((pac = (wchar_t *)malloc(acsize)) == 0)
		return (EOF);
	for (stat = 0; 0 < m; pc += n, ++px->n0, --m)
		{	/* generate a wide character */
		if (acsize <= (size_t)px->n0)
			{	/* drain buffer */
			px->width = 0;
			if ((stat = _WPuttxt(px, pac)) < 0)
				break;
			px->n0 = 0;
			}
		if ((n = _Mbtowc(&pac[px->n0], pc, INT_MAX,
			&mbst)) < 0)
			{	/* stop on bad conversion */
			stat = EOF;
			break;
			}
		else if (n == 0 && pac[px->n0] == L'\0')
			break;
		}
	if (stat == 0)
		stat = _WPuttxt(px, pac);
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

__SRCVERSION("xwputstr.c $Rev: 153052 $");
