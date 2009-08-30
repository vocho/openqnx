/* iswdigit function */
#include <wctype.h>
_STD_BEGIN

int (iswdigit)(wint_t wc)
	{	/* test for digit wide character */
	return (_Iswctype(wc, 4));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("iswdigit.c $Rev: 153052 $");
