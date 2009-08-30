/* getc function */
#include "xstdio.h"
_STD_BEGIN

int (getc)(FILE *str)
	{	/* get a character from stream */
	return (fgetc(str));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("getc.c $Rev: 153052 $");
