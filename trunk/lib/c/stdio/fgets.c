/* fgets function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

char *(fgets)(char *_Restrict buf, int n, FILE *_Restrict str)
	{	/* get a line from stream */
	unsigned char *s = (unsigned char *)buf;

	if (n <= 1)
		return (0);
	--n;

	_Lockfileatomic(str);
	if ((str->_Mode & _MBYTE) != 0)
		for (; 0 < n
			&& str->_Rback < str->_Back + sizeof (str->_Back); --n)
			{	/* deliver pushed back chars */
			*s = *str->_Rback++;
			if (*s++ == '\n')
				n = 1;	/* terminate full line */
			}

	while (0 < n)
		{	/* ensure buffer has chars */
		if (str->_Rsave != 0)
			str->_Rend = str->_Rsave, str->_Rsave = 0;
		if (str->_Next < str->_Rend)
			;
		else if (_Frprep(str) < 0)
			{	/* nothing to read */
			_Unlockfileatomic(str);
			return (0);
			}
		else if (str->_Mode & _MEOF)
			break;

		 {	/* copy as many as possible */
		unsigned char *s1 =
			(unsigned char *)memchr((void *)str->_Next,
			'\n', str->_Rend - str->_Next);
		size_t m = (s1 ? s1 + 1 : str->_Rend) - str->_Next;

		if ((size_t)n < m)
			s1 = 0, m = n;
		memcpy(s, str->_Next, m);
		s += m, n -= m;
		str->_Next += m;
		if (s1 != 0)
			break;	/* full line, quit */
		 }
		}

	if (s == (unsigned char *)buf)
		buf = 0;	/* nothing read, report failure */
	else
		*s = '\0';	/* terminate partial line */
	_Unlockfileatomic(str);
	return (buf);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fgets.c $Rev: 153052 $");
