/* fsetpos function */
#include "xstdio.h"
_STD_BEGIN

int (fsetpos)(FILE *str, const fpos_t *p)
	{	/* set file position indicator for stream */
	int ans;

	_Lockfileatomic(str);
	ans = _Fspos(str, p, 0L, SEEK_SET);
	_Unlockfileatomic(str);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fsetpos.c $Rev: 153052 $");
