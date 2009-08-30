/* clearerr function */
#include "xstdio.h"
_STD_BEGIN

void (clearerr)(FILE *str)
	{	/* clear EOF and error indicators for a stream */
	_Lockfileatomic(str);
	if (str->_Mode & (_MOPENR | _MOPENW))
		str->_Mode &= ~(_MEOF | _MERR);
	_Unlockfileatomic(str);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("clearerr.c $Rev: 153052 $");
