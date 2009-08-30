/* isblank function */
#include <ctype.h>
_STD_BEGIN

int (isblank)(int c)
	{	/* test for blank character */
	return (_Getchrtype(c) & (_SP | _XB));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isblank.c $Rev: 153052 $");
