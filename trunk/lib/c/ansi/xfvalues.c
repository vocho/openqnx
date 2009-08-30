/*
 * $QNXtpLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */






/* values used by math functions -- IEEE 754 float version */
#ifdef __QNX__		/* This is used to put the constants in the READ-ONLY sections */
#define _FLOAT_DATA_IS_CONST	/* __QNX__ */
#endif
#include "xmath.h"
_STD_BEGIN

		/* macros */
#define NBITS	(16 + _FOFF)

 #if _D0 == 0
  #define INIT(w0)		{(w0), 0}
  #define INIT2(w0, w1)	{(w0), (w1)}

 #else /* _D0 == 0 */
  #define INIT(w0)		{0, (w0)}
  #define INIT2(w0, w1)	{(w1), (w0)}
 #endif /* _D0 == 0 */

		/* static data */
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _FDenorm = {INIT2(0, 1)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _FEps = {
	INIT(((_FBIAS - NBITS) - 1) << _FOFF)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _FInf = {INIT(_FMAX << _FOFF)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _FNan = {INIT((_FMAX << _FOFF)
	| (1 << (_FOFF - 1)))};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _FSnan = {INIT2(_FMAX << _FOFF, 1)};
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ _Dconst _FRteps = {
	INIT((_FBIAS - NBITS / 2) << _FOFF)};

#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ float _FXbig = (NBITS + 1) * 347L / 1000;
#ifdef _FLOAT_DATA_IS_CONST	/* __QNX__ */
const
#endif
/* extern const */ float _FZero = 0.0F;
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xfvalues.c $Rev: 153052 $");
