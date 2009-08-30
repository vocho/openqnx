/* islower function */
#include <ctype.h>
_STD_BEGIN

int (islower)(int c)
	{	/* test for lowercase character */
	return (_Getchrtype(c) & _LO);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("islower.c $Rev: 153052 $");
