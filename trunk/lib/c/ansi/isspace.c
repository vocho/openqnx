/* isspace function */
#include <ctype.h>
_STD_BEGIN

int (isspace)(int c)
	{	/* test for spacing character */
	return (_Getchrtype(c) & (_CN | _SP | _XS));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isspace.c $Rev: 153052 $");
