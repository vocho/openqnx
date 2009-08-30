/* wctype function */
#include <yvals.h>
#include <string.h>
#include "xmtloc.h"
#include "xwctype.h"
_STD_BEGIN

wctype_t (wctype)(const char *name)
	{	/* find classification for wide character */
	wctype_t n;
	_PWctab pwctype = *_TLS_DATA_PTR(_Wctype);

	for (n = 1; pwctype[n]._Name != 0; ++n)
		if (strcmp(pwctype[n]._Name, name) == 0)
			break;
	return (pwctype[n]._Name != 0 ? n : 0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wctype.c $Rev: 153052 $");
