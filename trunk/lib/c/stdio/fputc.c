/* fputc function */
#include "xstdio.h"
_STD_BEGIN

int (fputc)(int ci, FILE *str)
	{	/* put a character to stream */
	unsigned char c = (unsigned char)ci;

	_Lockfileatomic(str);
	if (str->_Next < str->_Wend)
		;
	else if (_Fwprep(str) < 0)
		{	/* noplace to write */
		_Unlockfileatomic(str);
		return (EOF);
		}

	*str->_Next++ = c;
	if (((str->_Mode & _MNBF) != 0
		|| (str->_Mode & _MLBF) != 0 && c == '\n')
		&& fflush(str))
		{	/* write failed */
		_Unlockfileatomic(str);
		return (EOF);
		}

 #if !_MULTI_THREAD || !_FILE_OP_LOCKS
	if ((str->_Mode & (_MNBF| _MLBF)) != 0)
		str->_Wend = str->_Next;	/* disable buffering */
 #endif /* !_MULTI_THREAD || !_FILE_OP_LOCKS */

	_Unlockfileatomic(str);
	return (c);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fputc.c $Rev: 153052 $");
