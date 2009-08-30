/* strcpy function */
#include <string.h>
_STD_BEGIN

char *(strcpy)(char *_Restrict s1, const char *_Restrict s2)
	{	/* copy char s2[] to s1[] */
	char *s;

	for (s = s1; (*s++ = *s2++) != '\0'; )
		;
	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strcpy.c $Rev: 153052 $");
