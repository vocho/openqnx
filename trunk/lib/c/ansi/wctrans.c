/* wctrans function */
#include <string.h>
#include "xmtloc.h"
#include "xwctype.h"
_STD_BEGIN

wctrans_t (wctrans)(const char *name)
	{	/* find transformation for wide character */
	wctrans_t n;
	_PWctab pwctrans = *_TLS_DATA_PTR(_Wctrans);

	for (n = 1; pwctrans[n]._Name != 0; ++n)
		if (strcmp(pwctrans[n]._Name, name) == 0)
			return (n);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wctrans.c $Rev: 153052 $");
