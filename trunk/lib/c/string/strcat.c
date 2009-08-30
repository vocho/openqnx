/* strcat function */
#include <string.h>
_STD_BEGIN

char *(strcat)(char *_Restrict s1, const char *_Restrict s2)
	{	/* copy char s2[] to end of s1[] */
	char *s;

	for (s = s1; *s != '\0'; ++s)
		;			/* find end of s1[] */
	for (; (*s = *s2) != '\0'; ++s, ++s2)
		;			/* copy s2[] to end */
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strcat.c $Rev: 153052 $");
