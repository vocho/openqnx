/* setbuf function */
#include "xstdio.h"
_STD_BEGIN

void (setbuf)(FILE *_Restrict str, char *_Restrict buf)
	{	/* set up buffer for a stream */
	setvbuf(str, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("setbuf.c $Rev: 153052 $");
