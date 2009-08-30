/* _WStof function */
#include <wchar.h>
#include "xmath.h"
#include "xxwctype.h"
#include "xxfftype.h"

_STD_BEGIN

FTYPE _WStofx(const CTYPE *s, CTYPE **endptr, long pten, int *perr)
	#include "xxstod.h"

FTYPE _WStof(const CTYPE *s, CTYPE **endptr, long pten)
	{	/* convert string, discard error code */
	return (_WStofx(s, endptr, pten, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwstof.c $Rev: 200565 $");
