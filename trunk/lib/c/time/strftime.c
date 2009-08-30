/* strftime function */
#include "xmtloc.h"
#include "xtime.h"
_STD_BEGIN

size_t (strftime)(char *_Restrict s, size_t n,
	const char *_Restrict fmt, const struct tm *_Restrict t)
	{	/* format time to string */
	return (_CStrftime(s, n, fmt, t, _TLS_DATA_PTR(_Times)));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strftime.c $Rev: 153052 $");
