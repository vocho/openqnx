/* mblen function */
#include <stdlib.h>
#include "xwchar.h"
_STD_BEGIN

int (mblen)(const char *s, size_t n)
	{	/*	determine length of next multibyte code */
	int i;
	_Mbstinit(mbst);

	i = _Mbtowc(0, s, n, &mbst);
	return (i < 0 ? -1 : i);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mblen.c $Rev: 153052 $");
