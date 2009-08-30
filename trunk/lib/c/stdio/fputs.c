/* fputs function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

int (fputs)(const char *_Restrict s, FILE *_Restrict str)
	{	/* put a string to stream */
	_Lockfileatomic(str);
	while (*s != '\0')
		{	/* ensure room in buffer */
		if (str->_Next < str->_Wend)
			;
		else if (_Fwprep(str) < 0)
			{	/* noplace to write */
			_Unlockfileatomic(str);
			return (EOF);
			}

		 {	/* copy in as many as possible */
		const char *s1 = str->_Mode & _MLBF
			? strrchr(s, '\n') : 0;
		size_t m = s1 != 0 ? (s1 - s) + 1 : strlen(s);
		size_t n;

		n = str->_Wend - str->_Next;
		if (n < m)
			s1 = 0, m = n;
		memcpy(str->_Next, s, m);
		s += m;
		str->_Next += m;
		if (s1 != 0 && fflush(str))
			{	/* write failed */
			_Unlockfileatomic(str);
			return (EOF);
			}
		 }
		}

	if ((str->_Mode & _MNBF) != 0 && fflush(str))
		{	/* write failed */
		_Unlockfileatomic(str);
		return (EOF);
		}

 #if !_MULTI_THREAD || !_FILE_OP_LOCKS
	if ((str->_Mode & (_MNBF| _MLBF)) != 0)
		str->_Wend = str->_Next;	/* disable buffering */
 #endif /* !_MULTI_THREAD || !_FILE_OP_LOCKS */

	_Unlockfileatomic(str);
	return (0);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fputs.c $Rev: 153052 $");
