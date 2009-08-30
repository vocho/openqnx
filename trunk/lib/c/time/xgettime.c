/* _Gettime function */
#include <string.h>
#include "xtime.h"
_STD_BEGIN

const char *_Gettime(const char *s, int n, int *len)
	{	/*	get time info from environment */
	const char delim = (char)(*s ? *s++ : '\0');
	const char *s1;

	for (; ; --n, s = s1 + 1)
		{	/* find end of current field */
		if ((s1 = strchr(s, delim)) == 0)
			s1 = s + strlen(s);
		if (n <= 0)
			{	/* found proper field */
			*len = s1 - s;
			return (s);
			}
		else if (*s1 == '\0')
			{	/* not enough fields */
			*len = 0;
			return (s1);
			}
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xgettime.c $Rev: 153052 $");
