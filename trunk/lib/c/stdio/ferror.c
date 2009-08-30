/* ferror function */
#include "xstdio.h"
_STD_BEGIN

int (ferror)(FILE *str)
	{	/* test error indicator for a stream */
	int mode;

	_Lockfileatomic(str);
	mode = str->_Mode & _MERR;
	_Unlockfileatomic(str);
	return (mode);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("ferror.c $Rev: 153052 $");
