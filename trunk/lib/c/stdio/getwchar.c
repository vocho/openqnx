/* getwchar function */
#include "xwstdio.h"
_STD_BEGIN

wint_t (getwchar)(void)
	{	/* get a wchar_t from wide stdin */
	return (fgetwc(stdin));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("getwchar.c $Rev: 153052 $");
