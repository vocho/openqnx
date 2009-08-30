/* isprint function */
#include <ctype.h>
_STD_BEGIN

int (isprint)(int c)
	{	/* test for printable character */
	return (_Getchrtype(c) & (_DI | _LO | _PU | _SP | _UP | _XA));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("isprint.c $Rev: 153052 $");
