/* feof function */
#include "xstdio.h"
_STD_BEGIN

int (feof)(FILE *str)
	{	/* test end-of-file indicator for a stream */
	int mode;

	_Lockfileatomic(str);
	mode = str->_Mode & _MEOF;
	_Unlockfileatomic(str);
	return (mode);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("feof.c $Rev: 153052 $");
