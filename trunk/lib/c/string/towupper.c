/* towupper function */
#include <wctype.h>
#include "xlocale.h"
_STD_BEGIN

wint_t (towupper)(wint_t wc)
	{	/* convert to upper case wide character */
	return (_Towctrans(wc, 2));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("towupper.c $Rev: 153052 $");
