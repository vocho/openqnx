/* putchar function */
#include "xstdio.h"
_STD_BEGIN

int (putchar)(int c)
	{	/* put character to stdout */
	return (fputc(c, stdout));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("putchar.c $Rev: 153052 $");
