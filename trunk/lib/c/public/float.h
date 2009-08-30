/*
 * $QNXtpLicenseC:  
 * Copyright 2006, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/



/* float.h standard header -- IEEE 754 version */
#ifndef _FLOAT
#define _FLOAT
#ifndef _YVALS
 #include <yvals.h>
#endif

_C_STD_BEGIN

		/* TYPE DEFINITIONS */
 #ifndef _DVALS
  #define _DVALS
typedef struct
	{	/* parameters for a floating-point type */
	int _Ddig, _Dmdig, _Dmax10e, _Dmaxe, _Dmin10e, _Dmine;
	union
		{	/* union of short array and all floats */
		unsigned short _Us[8];
		float _Float;
		double _Double;
		long double _Long_double;
		} _Deps, _Dmax, _Dmin;
	} _Dvals;
 #endif /* _DVALS */

		/* DECLARATIONS */
_C_LIB_DECL
int _Fltrounds(void);
#ifdef _FLOAT_DATA_IS_CONST
#if defined(__SLIB_DATA_INDIRECT) && !defined(_Dbl) && !defined(__SLIB)
  extern const _Dvals *__get_Dbl_ptr(void);
  #define _Dbl  (*__get_Dbl_ptr())
#else
extern const _Dvals _Dbl;
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(_Flt) && !defined(__SLIB)
  extern const _Dvals *__get_Flt_ptr(void);
  #define _Flt  (*__get_Flt_ptr())
#else
extern const _Dvals _Flt;
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(_Ldbl) && !defined(__SLIB)
  extern const _Dvals *__get_Ldbl_ptr(void);
  #define _Ldbl (*__get_Ldbl_ptr())
#else
extern const _Dvals _Ldbl;
#endif

#else /* not _FLOAT_DATA_IS_CONST */
#if defined(__SLIB_DATA_INDIRECT) && !defined(_Dbl) && !defined(__SLIB)
  extern _Dvals *__get_Dbl_ptr(void);
  #define _Dbl  (*__get_Dbl_ptr())
#else
extern _Dvals _Dbl;
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(_Flt) && !defined(__SLIB)
  extern _Dvals *__get_Flt_ptr(void);
  #define _Flt  (*__get_Flt_ptr())
#else
extern _Dvals _Flt;
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(_Ldbl) && !defined(__SLIB)
  extern _Dvals *__get_Ldbl_ptr(void);
  #define _Ldbl (*__get_Ldbl_ptr())
#else
extern _Dvals _Ldbl;
#endif
#endif
_END_C_LIB_DECL

		/* COMMON PROPERTIES */
#define FLT_RADIX		2
#define FLT_ROUNDS		(_CSTD _Fltrounds())

 #if _HAS_C9X

 #ifdef _FEVAL
  #define FLT_EVAL_METHOD	_FEVAL

 #else /* _FEVAL */
  #define FLT_EVAL_METHOD	-1	/* indeterminable */
 #endif /* _FEVAL */

 #if _DLONG == 0
  #define DECIMAL_DIG	17	/* 64-bit long double */

 #elif _DLONG == 1
  #define DECIMAL_DIG	21	/* 80-bit long double */

 #else /* 1 < _DLONG */
  #define DECIMAL_DIG	36	/* 128-bit SPARC long double */
 #endif /* _DLONG */

 #endif /* _IS_C9X */

		/* float PROPERTIES */

 #if 199901L <= __STDC_VERSION__ && !defined(__APPLE__)

 /* __QNX__: original dinkum code didn't have the !defined(__GNUC__) clause */
 #if defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 )
	/* IEEE 4 byte */
  #define FLT_EPSILON		_CSTD _Flt._Deps._Float
  #define FLT_MAX			_CSTD _Flt._Dmax._Float
  #define FLT_MIN			_CSTD _Flt._Dmin._Float

 #else /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */
  #define FLT_EPSILON		0x8p-26
  #define FLT_MAX			0xf.fffffp+124
  #define FLT_MIN			0x8p-129
 #endif /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */

 #else /* 199901L <= __STDC_VERSION__ */
 #define FLT_EPSILON		_CSTD _Flt._Deps._Float
 #define FLT_MAX			_CSTD _Flt._Dmax._Float
 #define FLT_MIN			_CSTD _Flt._Dmin._Float
 #endif /* 199901L <= __STDC_VERSION__ */

#define FLT_DIG			6	/* _CSTD _Flt._Ddig */
#define FLT_MANT_DIG	24	/* _CSTD _Flt._Dmdig */
#define FLT_MAX_10_EXP	38	/* _CSTD _Flt._Dmax10e */
#define FLT_MAX_EXP		128	/* _CSTD _Flt._Dmaxe */
#define FLT_MIN_10_EXP	-37	/* _CSTD _Flt._Dmin10e */
#define FLT_MIN_EXP		-125	/* _CSTD _Flt._Dmine */

		/* double PROPERTIES */

 #if 199901L <= __STDC_VERSION__

 /* __QNX__: original dinkum code didn't have the !defined(__GNUC__) clause */
 #if  defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 )
	/* IEEE 8 byte */
  #define DBL_EPSILON		_CSTD _Dbl._Deps._Double
  #define DBL_MAX			_CSTD _Dbl._Dmax._Double
  #define DBL_MIN			_CSTD _Dbl._Dmin._Double

 #else /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */
  #define DBL_EPSILON		0x8p-55
  #define DBL_MAX			0xf.ffffffffffff8p+1020
  #define DBL_MIN			0x8p-1025
 #endif /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */

 #else /* 199901L <= __STDC_VERSION__ */
 #define DBL_EPSILON		_CSTD _Dbl._Deps._Double
 #define DBL_MAX			_CSTD _Dbl._Dmax._Double
 #define DBL_MIN			_CSTD _Dbl._Dmin._Double
 #endif /* 199901L <= __STDC_VERSION__ */

#define DBL_DIG			15 	/* _CSTD _Dbl._Ddig */
#define DBL_MANT_DIG	53	/* _CSTD _Dbl._Dmdig */
#define DBL_MAX_10_EXP	308	/* _CSTD _Dbl._Dmax10e */
#define DBL_MAX_EXP		1024	/* _CSTD _Dbl._Dmaxe */
#define DBL_MIN_10_EXP	-307	/* _CSTD _Dbl._Dmin10e */
#define DBL_MIN_EXP		-1021	/* _CSTD _Dbl._Dmine */

		/* long double PROPERTIES */

 #if _DLONG == 0

 #if 199901L <= __STDC_VERSION__

  /* __QNX__: original dinkum code didn't have the !defined(__GNUC__) clause */
  #if defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 )
	/* IEEE 8 byte */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double

  #else /* defined(__cplusplus) || ( defined(__GNUC__) && __GNUC__ < 3 ) */
   #define LDBL_EPSILON	0x8p-55L
   #define LDBL_MAX		0xf.ffffffffffff8p+1020L
   #define LDBL_MIN		0x8p-1025L
  #endif /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */

 #else /* 199901L <= __STDC_VERSION__ */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
 #endif /* 199901L <= __STDC_VERSION__ */

  #define LDBL_DIG			15	/* _CSTD _Ldbl._Ddig */
  #define LDBL_MANT_DIG		53	/* _CSTD _Ldbl._Dmdig */
  #define LDBL_MAX_10_EXP	308	/* _CSTD _Ldbl._Dmax10e */
  #define LDBL_MAX_EXP		1024	/* _CSTD _Ldbl._Dmaxe */
  #define LDBL_MIN_10_EXP	-307	/* _CSTD _Ldbl._Dmin10e */
  #define LDBL_MIN_EXP		-1021	/* _CSTD _Ldbl._Dmine */

 #elif _DLONG == 1

 #if 199901L <= __STDC_VERSION__

  /* __QNX__: original dinkum code didn't have the !defined(__GNUC__) clause */
  #if defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 )
	/* IEEE 10 byte, no hidden bit */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double

  #else /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */
   #define LDBL_EPSILON	0x8p-66L
   #define LDBL_MAX		0xf.fffffffffffffffp+16380L
   #define LDBL_MIN		0x8p-16385L
  #endif /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */

 #else /* 199901L <= __STDC_VERSION__ */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
 #endif /* 199901L <= __STDC_VERSION__ */

  #define LDBL_DIG			18	/* _CSTD _Ldbl._Ddig */
  #define LDBL_MANT_DIG		64	/* _CSTD _Ldbl._Dmdig */
  #define LDBL_MAX_10_EXP	4932	/* _CSTD _Ldbl._Dmax10e */
  #define LDBL_MAX_EXP		16384	/* _CSTD _Ldbl._Dmaxe */
  #define LDBL_MIN_10_EXP	-4931	/* _CSTD _Ldbl._Dmin10e */
  #define LDBL_MIN_EXP		-16381	/* _CSTD _Ldbl._Dmine */

 #else /* 1 < _DLONG */

 #if 199901L <= __STDC_VERSION__
	/* IEEE 16 byte, hidden bit */

  /* __QNX__: original dinkum code didn't have the !defined(__GNUC__) clause */
  #if defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 )
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double

  #else /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */
   #define LDBL_EPSILON	0x8p-115L
   #define LDBL_MAX		0xf.fffffffffffffffffffffffffff8p+16380L

   #if !defined(__EDG__) || 245 < __EDG_VERSION__
    #define LDBL_MIN		0x8p-16385L

   #else /* !defined(__EDG__) || 245 < __EDG_VERSION__ */
    #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
   #endif /* !defined(__EDG__) || 245 < __EDG_VERSION__ */

  #endif /* defined(__cplusplus) || !defined(__GNUC__) || ( defined(__GNUC__) && __GNUC__ < 3 ) */

 #else /* 199901L <= __STDC_VERSION__ */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
 #endif /* 199901L <= __STDC_VERSION__ */

  #define LDBL_DIG			33	/* _CSTD _Ldbl._Ddig */
  #define LDBL_MANT_DIG		113	/* _CSTD _Ldbl._Dmdig */
  #define LDBL_MAX_10_EXP	4932	/* _CSTD _Ldbl._Dmax10e */
  #define LDBL_MAX_EXP		16384	/* _CSTD _Ldbl._Dmaxe */
  #define LDBL_MIN_10_EXP	-4931	/* _CSTD _Ldbl._Dmin10e */
  #define LDBL_MIN_EXP		-16381	/* _CSTD _Ldbl._Dmine */
 #endif	/* _DLONG */

_C_STD_END
#endif /* _FLOAT */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("float.h $Rev: 154119 $"); */
