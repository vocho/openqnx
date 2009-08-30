/* difftime function */
#include "xtime.h"
_STD_BEGIN

double (difftime)(time_t t1, time_t t0)
	{	/* compute difference in times */
#if !defined(__QNX__)
	t0 -= _TBIAS, t1 -= _TBIAS;
#endif
	return (t0 <= t1 ? (double)(t1 - t0) : -(double)(t0 - t1));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("difftime.c $Rev: 153052 $");
