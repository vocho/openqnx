/* _Stod function */
#include <stdlib.h>
#include "xmath.h"
#include "xxcctype.h"
#include "xxdftype.h"

_STD_BEGIN

FTYPE _Stodx(const CTYPE *s, CTYPE **endptr, long pten, int *perr)
	#include "xxstod.h"

FTYPE _Stod(const CTYPE *s, CTYPE **endptr, long pten)
	{	/* convert string, discard error code */
	return (_Stodx(s, endptr, pten, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xstod.c $Rev: 200565 $");
