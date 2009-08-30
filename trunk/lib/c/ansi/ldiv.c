/* ldiv function */
#include <stdlib.h>
_STD_BEGIN

ldiv_t (ldiv)(long numer, long denom)
	{	/* compute long quotient and remainder */
	ldiv_t val;
	static const int fixneg = -1 / 2;

	val.quot = numer / denom;
	val.rem = numer - denom * val.quot;
	if (fixneg < 0 && val.quot < 0 && val.rem != 0)
		{	/* fix incorrect truncation */
		val.quot += 1;
		val.rem -= denom;
		}
	return (val);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("ldiv.c $Rev: 153052 $");
