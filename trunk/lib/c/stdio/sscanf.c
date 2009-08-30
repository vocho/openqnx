/* sscanf function */
#include "xstdio.h"
_STD_BEGIN

static int scin(void *str, int ch, int getfl)
	{	/* get or put a character */
	unsigned char *s = *(unsigned char **)str;

	if (!getfl)
		{	/* back up a char */
		*(unsigned char **)str = s - 1;
		return (ch);
		}
	else if (*s == '\0')
		return (EOF);
	else
		{	/* deliver a char */
		*(unsigned char **)str = s + 1;
		return (*s);
		}
	}

int (sscanf)(const char *_Restrict buf, const char *_Restrict fmt, ...)
	{	/* read formatted from string */
	int ans;
	va_list ap;

	va_start(ap, fmt);
	ans = _Scanf(&scin, (void **)&buf, fmt, ap, 0);
	va_end(ap);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("sscanf.c $Rev: 153052 $");
