/* towctrans function */
#include <wctype.h>
_STD_BEGIN

wint_t (towctrans)(wint_t wc, wctrans_t off)
	{	/* external wrapper */
	return (_Towctrans(wc, off));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("towctran.c $Rev: 153052 $");
