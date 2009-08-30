/* mbsinit function */
#include <wchar.h>
_STD_BEGIN

int (mbsinit)(const mbstate_t *pst)
	{	/* test for initial state */
	return (pst == 0 || pst->_State == 0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mbsinit.c $Rev: 153052 $");
