/* getchar function */
#include "xstdio.h"
_STD_BEGIN

int (getchar)(void)
	{	/* get a character from stdin */
	return (fgetc(stdin));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("getchar.c $Rev: 153052 $");
