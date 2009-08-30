/* gets function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

char *(gets)(char *buf)
	{	/* get a line from stdio */
	unsigned char *s = (unsigned char *)buf;

	_Lockfileatomic(stdin);
	if ((stdin->_Mode & _MBYTE) != 0)
		for (; stdin->_Rback < stdin->_Back + sizeof (stdin->_Back); )
			{	/* deliver pushed back chars */
			*s = *stdin->_Rback++;
			if (*s++ == '\n')
				{	/* terminate full line */
				s[-1] = '\0';
				_Unlockfileatomic(stdin);
				return (buf);
				}
			}

	for (; ; )
		{	/* ensure chars in buffer */
		if (stdin->_Rsave != 0)
			stdin->_Rend = stdin->_Rsave, stdin->_Rsave = 0;
		if (stdin->_Next < stdin->_Rend)
			;
		else if (_Frprep(stdin) < 0)
			{	/* nothing to read */
			_Unlockfileatomic(stdin);
			return (0);
			}
		else if (stdin->_Mode & _MEOF)
			break;

		 {	/* deliver as many as possible */
		unsigned char *s1
			= (unsigned char *)memchr((void *)stdin->_Next,
				'\n', stdin->_Rend - stdin->_Next);
		size_t m = (s1 ? s1 + 1 : stdin->_Rend) - stdin->_Next;

		memcpy(s, stdin->_Next, m);
		s += m; stdin->_Next += m;
		if (s1 != 0)
			{	/* terminate full line */
			s[-1] = '\0';
			_Unlockfileatomic(stdin);
			return (buf);
			}
		 }
		}

	if (s == (unsigned char *)buf)
		buf = 0;
	else
		*s = '\0';
	_Unlockfileatomic(stdin);
	return (buf);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("gets.c $Rev: 153052 $");
