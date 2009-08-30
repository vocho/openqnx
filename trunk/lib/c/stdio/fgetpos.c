/* fgetpos function */
#include "xstdio.h"
_STD_BEGIN

int (fgetpos)(FILE *_Restrict str, fpos_t *_Restrict p)
	{	/* get file position indicator for stream */
	int ans;

	_Lockfileatomic(str);
	ans = _Fgpos(str, p);
	_Unlockfileatomic(str);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fgetpos.c $Rev: 153052 $");
