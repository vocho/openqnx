/* isxdigit function */
#include <ctype.h>
_STD_BEGIN

int (isxdigit)(int c)
	{	/* test for hexadecimal digit */
	return (_Getchrtype(c) & _XD);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isxdigit.c $Rev: 153052 $");
