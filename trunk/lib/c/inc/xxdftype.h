/* xxdftype.h -- parameters for double floating-point type */
#include <float.h>

#define FTYPE	double
#define FCTYPE	_Dcomplex
#define FBITS	DBL_MANT_DIG
#define FEPS	DBL_EPSILON
#define FMAXEXP	DBL_MAX_EXP
#define FFUN(fun)	fun
#define FMACRO(x)	x
#define FNAME(fun)	_##fun
#define FCONST(obj)	_##obj._Double
#define FLIT(lit)	lit
#define FISNEG(exp)	DSIGN(exp)

  #ifdef __QNX__
#define FMIN		DBL_MIN
  #endif  

 #if _IS_EMBEDDED
#define FCPTYPE	double_complex

 #else /* _IS_EMBEDDED */
#define FCPTYPE	complex<double>
 #endif /* _IS_EMBEDDED */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */
