/* putwchar function */
#include "xwstdio.h"
_STD_BEGIN

wint_t (putwchar)(wchar_t c)
	{	/* put character to wide stdout */
	return (fputwc(c, stdout));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("putwchar.c $Rev: 153052 $");
