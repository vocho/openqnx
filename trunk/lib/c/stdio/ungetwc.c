/* ungetwc function */
#include "xwstdio.h"
_STD_BEGIN

wint_t (ungetwc)(wint_t c, FILE *str)
	{	/* push character back on wide stream */
	_Lockfileatomic(str);
	if (c == WEOF
		|| str->_WRback <= str->_WBack
		|| (str->_Mode & (_MOPENR | _MWRITE | _MBYTE))
			!= _MOPENR)
		c = WEOF;
	else
		{	/* pushback permitted, do it */
		str->_Mode =
			(unsigned short)(str->_Mode & ~_MEOF | (_MREAD | _MWIDE));
		*--str->_WRback = c;
		}
	_Unlockfileatomic(str);
	return (c);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("ungetwc.c $Rev: 153052 $");
