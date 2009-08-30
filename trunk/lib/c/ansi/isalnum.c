/* isalnum function */
#include <ctype.h>
_STD_BEGIN

int (isalnum)(int c)
	{	/* test for alphanumeric character */
	return (_Getchrtype(c) & (_DI | _LO | _UP | _XA));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isalnum.c $Rev: 153052 $");
