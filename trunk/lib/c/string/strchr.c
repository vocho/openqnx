/* strchr function */
#include <string.h>
_STD_BEGIN

_Const_return char *(strchr)(const char *s, int c)
	{	/* find first occurrence of c in char s[] */
	const char ch = (char)c;

	for (; *s != ch; ++s)
		if (*s == '\0')
			return (0);
	return ((char *)s);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strchr.c $Rev: 153052 $");
