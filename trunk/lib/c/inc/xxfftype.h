/* xxfftype.h -- parameters for float floating-point type */
#include <float.h>

#define FTYPE	float
#define FCTYPE	_Fcomplex
#define FBITS	FLT_MANT_DIG
#define FEPS	FLT_EPSILON
#define FMAXEXP	FLT_MAX_EXP
#define FFUN(fun)	fun##f
#define FMACRO(x)	F##x
#define FNAME(fun)	_F##fun
#define FCONST(obj)	_F##obj._Float
#define FLIT(lit)	lit##F
#define FISNEG(exp)	FSIGN(exp)

  #ifdef __QNX__
#define FMIN		FLT_MIN
  #endif

 #if _IS_EMBEDDED
#define FCPTYPE	float_complex

 #else /* _IS_EMBEDDED */
#define FCPTYPE	complex<float>
 #endif /* _IS_EMBEDDED */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */
