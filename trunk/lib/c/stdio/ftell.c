/* ftell function */
#include "xstdio.h"
_STD_BEGIN

long (ftell)(FILE *str)
	{	/* get seek offset for stream */
	int ans;

	_Lockfileatomic(str);
	ans = _Fgpos(str, 0);
	_Unlockfileatomic(str);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("ftell.c $Rev: 153052 $");
