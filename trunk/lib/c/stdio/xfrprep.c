/* _Frprep function */
#include <stdlib.h>

#ifdef __QNX__
#include <errno.h>
#endif

#include "xstdio.h"
#include "yfuns.h"
_STD_BEGIN

int _Frprep(FILE *str)
	{	/* prepare stream for reading */

 #ifdef _WIN32_WCE
	if (str->_Handle == 0
		&& str == &_Stdin
		&& (str->_Handle = _Fopen(_SYSCH("stdin.txt"), str->_Mode, 0)) == 0)
		return (-1);
 #endif /* _WIN32_WCE */

	if (str->_Next < str->_Rend)
		return (1);
	else if (str->_Mode & _MEOF)
		return (0);
	else if ((str->_Mode & (_MOPENR | _MWRITE | _MWIDE)) != _MOPENR)
		{	/* can't read after write */
		str->_Mode |= str->_Mode & _MWIDE
			? _MERR : _MERR | _MBYTE;
#ifdef __QNX__
		errno=EBADF; /* PR 12857 */
#endif
		return (-1);
		}
	if ((str->_Mode & _MNBF) != 0
		|| str->_Buf != &str->_Cbuf)
		;
#ifdef __QNX__
	else
		(void)_Fbuf(str);
#else
	else if ((str->_Buf = (unsigned char *)malloc(BUFSIZ)) == 0)
		{	/* use 1-char _Cbuf */
		str->_Buf = &str->_Cbuf;
		str->_Bend = str->_Buf + 1;
		}
	else
		{	/* use allocated buffer */
		str->_Mode |= _MALBUF;
		str->_Bend = str->_Buf + BUFSIZ;
		str->_WRend = str->_Buf;
		str->_WWend = str->_Buf;
		}
#endif
	str->_Next = str->_Buf;
	str->_Rend = str->_Buf;
	str->_Wend = str->_Buf;
#ifdef __QNX__
	if((str->_Mode & _MISTTY) && (str->_Mode & (_MNBF | _MLBF)))
		{	/* flush all interactive buffers */
		FILE *fp;

		for (fp = _Files[0]; fp; fp = fp->_NextFile)
			if ((fp->_Mode & (_MISTTY | _MWRITE)) == (_MISTTY | _MWRITE))
				fflush(fp);
		}
#endif
	 {	/* try to read into buffer */
	int n = _Fread(str, str->_Buf, str->_Bend - str->_Buf);

	if (n < 0)
		{	/* report error and fail */
		str->_Mode |= _MERR | _MBYTE;
		return (-1);
		}
	else if (n == 0)
		{	/* report end of file */
		str->_Mode =
			(unsigned short)((str->_Mode & ~_MREAD) | _MEOF | _MBYTE);
		return (0);
		}
	else
		{	/* set up data read */
		str->_Mode |= _MREAD | _MBYTE;
		str->_Rend += n;
		return (1);
		}
	 }
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xfrprep.c $Rev: 153052 $");
