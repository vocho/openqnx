/* wctob function */
#include <wchar.h>
_STD_BEGIN

int (wctob)(wint_t wc)
	{	/* translate wint_t to one-byte multibyte */
	return (_Wctob(wc));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wctob.c $Rev: 153052 $");
