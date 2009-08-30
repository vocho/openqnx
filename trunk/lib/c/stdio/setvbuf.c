/* setvbuf function */
#include <stdlib.h>
#include "xstdio.h"
_STD_BEGIN

int (setvbuf)(FILE *_Restrict str, char *_Restrict abuf,
	int smode, size_t size)
	{	/* set up buffer for a stream */
	int mode;
	unsigned char *buf = (unsigned char *)abuf;

	mode = smode == _IOFBF ? 0
		: smode == _IOLBF ? _MLBF
		: smode == _IONBF ? _MNBF : -1;
	if (mode == -1)
		return (-1);

	_Lockfileatomic(str);
	if (str->_Mode & (_MREAD | _MWRITE))
		{	/* file operation has already occurred */
		_Unlockfileatomic(str);
		return (-1);
		}

	if (INT_MAX < size)
		size = INT_MAX;	/* trim to largest usable size */
	else if (0 < size)
		;
	else if (buf == 0)
		size = BUFSIZ;	/* allocate default size if unspecified */
	else
		{	/* supplied buffer too small, use default */
		buf = &str->_Cbuf;
		size = 1;
		mode = _MNBF;
		}

	if (buf != 0)
		;
	else if ((buf = (unsigned char *)malloc(size)) == 0)
		{	/* can't allocate space */
		_Unlockfileatomic(str);
		return (-1);
		}
	else
		mode |= _MALBUF;

	if (str->_Mode & _MALBUF)
		free(str->_Buf), str->_Mode &= ~_MALBUF;
	str->_Mode = (unsigned short)(str->_Mode & ~(_MLBF | _MNBF) | mode);
	str->_Buf = buf;
	str->_Bend = buf + size;
	str->_Next = buf;
	str->_Rend = buf, str->_WRend = buf;
	str->_Wend = buf, str->_WWend = buf;
#ifdef __QNXNTO__
	if ((str->_Mode & _MISTTY) == 0 && isatty(fileno(str)))
		str->_Mode |= _MISTTY;
#endif
	_Closreg();
	_Unlockfileatomic(str);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("setvbuf.c $Rev: 153052 $");
