/* strcmp function */
#include <string.h>
_STD_BEGIN

int (strcmp)(const char *s1, const char *s2)
	{	/* compare unsigned char s1[], s2[] */
	for (; *s1 == *s2; ++s1, ++s2)
		if (*s1 == '\0')
			return (0);
	return (*(unsigned char *)s1 < *(unsigned char *)s2
		? -1 : +1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strcmp.c $Rev: 153052 $");
