/* getwc function */
#include "xwstdio.h"
_STD_BEGIN

wint_t (getwc)(FILE *str)
	{	/* get a character from wide stream */
	return (fgetwc(str));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("getwc.c $Rev: 153052 $");
