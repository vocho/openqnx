/* iswblank function */
#include <wctype.h>
_STD_BEGIN

int (iswblank)(wint_t wc)
	{	/* test for space or horizontal tab wide character */
	return (_Iswctype(wc, 12));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("iswblank.c $Rev: 153052 $");
