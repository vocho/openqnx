/* fgetc function */
#include "xstdio.h"
_STD_BEGIN

int (fgetc)(FILE *str)
	{	/* get a character from stream */
	int ch;

	_Lockfileatomic(str);
	if (str->_Rback < str->_Back + sizeof (str->_Back)
		&& (str->_Mode & _MBYTE) != 0)
		{	/* deliver putback character */
		ch = *str->_Rback++;
		_Unlockfileatomic(str);
		return (ch);
		}

	if (str->_Rsave != 0)
		str->_Rend = str->_Rsave, str->_Rsave = 0;
	if (str->_Next < str->_Rend)
		ch = *str->_Next++;
	else if (_Frprep(str) <= 0)
		ch = EOF;
	else
		ch = *str->_Next++;
	_Unlockfileatomic(str);
	return (ch);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fgetc.c $Rev: 153052 $");
