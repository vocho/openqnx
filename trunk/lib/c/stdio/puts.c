/* puts function */
#include "xstdio.h"
_STD_BEGIN

int (puts)(const char *s)
	{	/*	put string + newline to stdout */
	int ans;

	_Lockfileatomic(stdout);
	ans = fputs(s, stdout) < 0 || fputc('\n', stdout) < 0 ? EOF : 0;
	_Unlockfileatomic(stdout);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("puts.c $Rev: 153052 $");
