/* _Fbuf function */
#include <stdlib.h>
#include <errno.h>
#include "xstdio.h"
_STD_BEGIN

int _Fbuf(FILE *str)
	{	/* allocate buffer */
	int			save_errno = errno;

	if (isatty(fileno(str)))
		str->_Mode |= _MISTTY | _MLBF;
	if ((str->_Buf = (unsigned char *)malloc(BUFSIZ)) == 0)
		{	/* use 1-char _Cbuf */
		str->_Buf = &str->_Cbuf;
		str->_Bend = str->_Buf + 1;
		}
	else
		{	/* use allocated buffer */
		str->_Mode |= _MALBUF;
		str->_Bend = str->_Buf + BUFSIZ;
		}
	str->_Next = str->_Buf;
	str->_Rend = str->_Buf;
	str->_Wend = str->_Buf;
	str->_WRend = str->_Buf;
	str->_WWend = str->_Buf;
	errno = save_errno;
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1994-2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

__SRCVERSION("xfbuf.c $Rev: 153052 $");
