/* _Wctob function */
#include <limits.h>
#include <stdio.h>
#include "xwchar.h"
_STD_BEGIN

int (_Wctob)(wint_t wc)
	{	/* internal function to translate wint_t */
	if (wc == WEOF)
		return (EOF);
	else
		{	/* translate wc into buffer */
		char buf[MB_LEN_MAX];
		_Mbstinit(mbst);

		return (_Wctomb(buf, wc, &mbst) != 1 ? EOF
			: (unsigned char)buf[0]);
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwctob.c $Rev: 153052 $");
