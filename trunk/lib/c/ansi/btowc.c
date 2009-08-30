/* btowc function */
#include <wchar.h>
_STD_BEGIN

wint_t (btowc)(int c)
	{	/* convert single byte to wide character */
	return (_Btowc(c));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("btowc.c $Rev: 153052 $");
