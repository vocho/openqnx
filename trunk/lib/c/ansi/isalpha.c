/* isalpha function */
#include <ctype.h>
_STD_BEGIN

int (isalpha)(int c)
	{	/* test for alphabetic character */
	return (_Getchrtype(c) & (_LO | _UP | _XA));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isalpha.c $Rev: 153052 $");
