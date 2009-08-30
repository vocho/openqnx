/* strrchr function */
#include <string.h>
_STD_BEGIN

_Const_return char *(strrchr)(const char *s, int c)
	{	/* find last occurrence of c in char s[] */
	const char ch = (char)c;
	const char *sc;

	for (sc = 0; ; ++s)
		{	/* check another char */
		if (*s == ch)
			sc = s;
		if (*s == '\0')
			return ((char *)sc);
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strrchr.c $Rev: 153052 $");
