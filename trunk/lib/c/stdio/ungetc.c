/* ungetc function */
#include "xstdio.h"
_STD_BEGIN

int (ungetc)(int c, FILE *str)
	{	/* push character back on stream */
	_Lockfileatomic(str);
	if (c == EOF
		|| str->_Rback <= str->_Back
		|| (str->_Mode & (_MOPENR | _MWRITE | _MWIDE))
			!= _MOPENR)
		c = EOF;
	else
		{	/* pushback permitted, do it */
		str->_Mode =
			(unsigned short)(str->_Mode & ~_MEOF | (_MREAD | _MBYTE));
		if (str->_Rsave == 0)
			str->_Rsave = str->_Rend, str->_Rend = str->_Buf;
		*--str->_Rback = (unsigned char)c;
		}
	_Unlockfileatomic(str);
	return (c);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("ungetc.c $Rev: 153052 $");
