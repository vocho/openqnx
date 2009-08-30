/* fwrite function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

size_t (fwrite)(const void *_Restrict ptr, size_t size,
	size_t nelem, FILE *_Restrict str)
	{	/* write to stream from array */
	char *s = (char *)ptr;
	size_t ns = size * nelem;
	size_t nleft = 0;

	if (ns == 0)
		return (0);
	else if (size == 0)
		return (nelem);

	_Lockfileatomic(str);
	while (0 < ns)
		{	/* ensure room in buffer */
		if (str->_Next < str->_Wend)
			;
		else if (_Fwprep(str) < 0)
			break;
		 {	/* copy in as many as possible */
		char *s1 = str->_Mode & _MLBF
			? (char *)memchr((void *)s, '\n', ns) : 0;
		size_t m = s1 ? (s1 - s) + 1 : ns;
		size_t n = str->_Wend - str->_Next;

		if (n < m)
			s1 = 0, m = n;
		memcpy(str->_Next, s, m);
		s += m, ns -= m;
		str->_Next += m;
		nleft = str->_Next - str->_Buf;
		if (s1 && fflush(str))
			break;
		nleft = 0;
		 }
		}
	if (str->_Mode & _MNBF && nleft == 0)
		{	/* unbuffered, flush any remaining output */
		nleft = str->_Next - str->_Buf;
		if (!fflush(str))
			nleft = 0;
		}
	ns += nleft;

 #if !_MULTI_THREAD || !_FILE_OP_LOCKS
	if ((str->_Mode & (_MNBF| _MLBF)) != 0)
		str->_Wend = str->_Next;	/* disable buffering */
 #endif /* !_MULTI_THREAD || !_FILE_OP_LOCKS */

	_Unlockfileatomic(str);
	return ((size * nelem - ns) / size);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fwrite.c $Rev: 153052 $");
