/* isdigit function */
#include <ctype.h>
_STD_BEGIN

int (isdigit)(int c)
	{	/* test for digit */
	return (_Getchrtype(c) & _DI);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isdigit.c $Rev: 153052 $");
