/* fread function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

size_t (fread)(void *_Restrict ptr, size_t size, size_t nelem,
	FILE *_Restrict str)
	{	/* read into array from stream */
	size_t ns = size * nelem;
	unsigned char *s = (unsigned char *)ptr;

	if (ns == 0)
		return (0);
	_Lockfileatomic(str);
	if ((str->_Mode & _MBYTE) != 0)
		for (; 0 < ns && str->_Rback < str->_Back + sizeof (str->_Back); --ns)
			*s++ = *str->_Rback++;
	while (0 < ns)
		{	/* ensure chars in buffer */
		if (str->_Rsave != 0)
			str->_Rend = str->_Rsave, str->_Rsave = 0;
		if (str->_Next < str->_Rend)
			;
		else if (_Frprep(str) <= 0)
			break;
		 {	/* deliver as many as possible */
		size_t m = str->_Rend - str->_Next;

		if (ns < m)
			m = ns;
		memcpy(s, str->_Next, m);
		s += m, ns -= m;
		str->_Next += m;
		 }
		}
	_Unlockfileatomic(str);
	return ((size * nelem - ns) / size);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fread.c $Rev: 153052 $");
