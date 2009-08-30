/* abs function */
#include <stdlib.h>
_STD_BEGIN

int (abs)(int i)
	{	/* compute absolute value of int argument */
	return (i < 0 ? -i : i);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("abs.c $Rev: 153052 $");
