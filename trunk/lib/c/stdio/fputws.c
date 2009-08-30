/* fputws function */
#include "xwstdio.h"
_STD_BEGIN

int (fputws)(const wchar_t *_Restrict s, FILE *_Restrict str)
	{	/* put a string to wide stream */
	_Lockfileatomic(str);
	for (; *s != '\0'; ++s)
		if (fputwc(*s, str) == WEOF)
			{	/* write failed */
			_Unlockfileatomic(str);
			return (EOF);
			}
	_Unlockfileatomic(str);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fputws.c $Rev: 153052 $");
