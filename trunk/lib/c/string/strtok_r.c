/* strtok function */
#include <string.h>
#include "xtls.h"
_STD_BEGIN

char *(strtok_r)(char *_Restrict s1, const char *_Restrict s2, char **s3)
	{	/* find next token in s1[] delimited by s2[] */
	char *sbegin, *send;

	sbegin = s1 ? s1 : *s3;
	sbegin += strspn(sbegin, s2);
	if (*sbegin == '\0')
		{	/* end of scan */
		*s3 = "";	/* for safety */
		return (0);
		}
	send = sbegin + strcspn(sbegin, s2);
	if (*send != '\0')
		*send++ = '\0';
	*s3 = send;
	return (sbegin);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strtok_r.c $Rev: 153052 $");
