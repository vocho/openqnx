/* _Dunscale function -- IEEE 754 version */
#include "xmath.h"
_STD_BEGIN

short _Dunscale(short *pex, double *px)
	{	/* separate *px to 1/2 <= |frac| < 1 and 2^*pex */
	unsigned short *ps = (unsigned short *)(char *)px;
	short xchar = (ps[_D0] & _DMASK) >> _DOFF;

	if (xchar == _DMAX)
		{	/* NaN or INF */
		*pex = 0;
		return ((ps[_D0] & _DFRAC) != 0 || ps[_D1] != 0
			|| ps[_D2] != 0 || ps[_D3] != 0 ? _NANCODE : _INFCODE);
		}
	else if (0 < xchar || (xchar = _Dnorm(ps)) <= 0)
		{	/* finite, reduce to [1/2, 1) */
		ps[_D0] = ps[_D0] & ~_DMASK | _DBIAS << _DOFF;
		*pex = xchar - _DBIAS;
		return (_FINITE);
		}
	else
		{	/* zero */
		*pex = 0;
		return (0);
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xdunscal.c $Rev: 153052 $");
