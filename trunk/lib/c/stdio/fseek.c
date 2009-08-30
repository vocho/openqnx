/* fseek function */
#include "xstdio.h"
_STD_BEGIN

int (fseek)(FILE *str, long off, int smode)
	{	/* set seek offset for stream */
	int ans;

	_Lockfileatomic(str);
	ans = _Fspos(str, 0, off, smode);
	_Unlockfileatomic(str);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fseek.c $Rev: 153052 $");
