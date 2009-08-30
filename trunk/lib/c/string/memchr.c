/* memchr function */
#include <string.h>
_STD_BEGIN

_Const_return void *(memchr)(const void *s, int c, size_t n)
	{	/* find first occurrence of c in s[n] */
	const unsigned char uc = (unsigned char)c;
	const unsigned char *su = (const unsigned char *)s;

	for (; 0 < n; ++su, --n)
		if (*su == uc)
			return ((void *)su);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("memchr.c $Rev: 153052 $");
