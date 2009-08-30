/* vswscanf function */
#include "xwstdio.h"
_STD_BEGIN

static wint_t scin(void *str, wint_t ch, int getfl)
	{	/* get or put a wide character */
	wchar_t *s = *(wchar_t **)str;

	if (!getfl)
		{	/* back up a wchar_t */
		*(wchar_t **)str = s - 1;
		return (ch);
		}
	else if (*s == L'\0')
		return (WEOF);
	else
		{	/* deliver a wchar_t */
		*(wchar_t **)str = s + 1;
		return (*s);
		}
	}

int (vswscanf)(const wchar_t *_Restrict buf,
	const wchar_t *_Restrict fmt, va_list ap)
	{	/* read formatted from wide string to arg list */
	return (_WScanf(&scin, (void **)&buf, fmt, ap, 0));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("vswscanf.c $Rev: 153052 $");
