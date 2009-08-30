/* iswprint function */
#include <wctype.h>
_STD_BEGIN

int (iswprint)(wint_t wc)
	{	/* test for printable wide character */
	return (_Iswctype(wc, 7));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("iswprint.c $Rev: 153052 $");
