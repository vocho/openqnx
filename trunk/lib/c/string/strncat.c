/* strncat function */
#include <string.h>
_STD_BEGIN

char *(strncat)(char *_Restrict s1, const char *_Restrict s2, size_t n)
	{	/* copy char s2[max n] to end of s1[] */
	char *s;

	for (s = s1; *s != '\0'; ++s)
		;	/* find end of s1[] */
	for (; 0 < n && *s2 != '\0'; --n)
		*s++ = *s2++;	/* copy at most n chars from s2[] */
	*s = '\0';
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strncat.c $Rev: 153052 $");
