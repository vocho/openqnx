/* towlower function */
#include <wctype.h>
#include "xlocale.h"
_STD_BEGIN

wint_t (towlower)(wint_t wc)
	{	/* convert to lower case wide character */
	return (_Towctrans(wc, 1));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("towlower.c $Rev: 153052 $");
