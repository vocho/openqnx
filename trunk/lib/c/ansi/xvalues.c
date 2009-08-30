/* values used by math functions -- IEEE 754 version */
#ifdef __QNX__		/* This is used to put the constants in the READ-ONLY sections */
#define _FLOAT_DATA_IS_CONST	/* __QNX__ */
#endif
#include "xmath.h"
_STD_BEGIN

		/* macros */
#define NBITS	(48 + _DOFF)

 #if _D0 == 0
  #define INIT(w0)		{(w0), 0, 0, 0}
  #define INIT2(w0, w1)	{(w0), 0, 0, (w1)}

 #else /* _D0 == 0 */
  #define INIT(w0)		{0, 0, 0, (w0)}
  #define INIT2(w0, w1)	{(w1), 0, 0, (w0)}
 #endif /* _D0 == 0 */

		/* static data */
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _Denorm = {INIT2(0, 1)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _Eps = {
	INIT(((_DBIAS - NBITS) - 1) << _DOFF)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _Hugeval = {INIT(_DMAX << _DOFF)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _Inf = {INIT(_DMAX << _DOFF)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _Nan = {INIT((_DMAX << _DOFF)
	| (1 << (_DOFF - 1)))};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _Snan = {INIT2(_DMAX << _DOFF, 1)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _Rteps = {
	INIT((_DBIAS - NBITS / 2) << _DOFF)};

#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ double _Xbig = (NBITS + 1) * 347L / 1000;
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ double _Zero = 0.0;
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xvalues.c $Rev: 153052 $");
