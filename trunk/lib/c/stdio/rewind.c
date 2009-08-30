/* rewind function */
#include "xstdio.h"
_STD_BEGIN

void (rewind)(FILE *str)
	{	/* rewind stream */
	_Lockfileatomic(str);
	(void)_Fspos(str, 0, 0L, SEEK_SET);
	str->_Mode &= ~_MERR;
	_Unlockfileatomic(str);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("rewind.c $Rev: 153052 $");
