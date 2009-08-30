/* isupper function */
#include <ctype.h>
_STD_BEGIN

int (isupper)(int c)
	{	/* test for uppercase character */
	return (_Getchrtype(c) & _UP);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isupper.c $Rev: 153052 $");
