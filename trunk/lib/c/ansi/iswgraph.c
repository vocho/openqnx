/* iswgraph function */
#include <wctype.h>
_STD_BEGIN

int (iswgraph)(wint_t wc)
	{	/* test for graphic wide character */
	return (_Iswctype(wc, 5));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("iswgraph.c $Rev: 153052 $");
