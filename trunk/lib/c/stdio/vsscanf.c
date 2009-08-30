/* vsscanf function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

static int scin(void *str, int ch, int getfl)
	{	/* get or put a character */
	char *s = *(char **)str;

	if (!getfl)
		{	/* back up a char */
		*(char **)str = s - 1;
		return (ch);
		}
	else if (*s == '\0')
		return (EOF);
	else
		{	/* deliver a char */
		*(char **)str = s + 1;
		return (*s);
		}
	}

int (vsscanf)(const char *_Restrict buf, const char *_Restrict fmt,
	va_list ap)
	{	/* read formatted from string to arg list */
	return (_Scanf(&scin, (void **)&buf, fmt, ap, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("vsscanf.c $Rev: 153052 $");
