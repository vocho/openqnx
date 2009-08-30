/* iscntrl function */
#include <ctype.h>
_STD_BEGIN

int (iscntrl)(int c)
	{	/* test for control character */
	return (_Getchrtype(c) & _BB);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("iscntrl.c $Rev: 153052 $");
