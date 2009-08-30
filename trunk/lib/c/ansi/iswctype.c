/* iswctype function */
#include <wctype.h>
_STD_BEGIN

int (iswctype)(wint_t wc, wctype_t off)
	{	/* external wrapper */
	return (_Iswctype(wc, off));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("iswctype.c $Rev: 153052 $");
