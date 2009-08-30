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





/* yvals.h values header for conforming compilers on various systems */
#ifndef _YVALS
#define _YVALS
#include <sys/platform.h>

#define _CPPLIB_VER	500

#ifdef __EXT_ANSIC_199901
#define _C99	1
#endif

#if defined(__GNUC__) && (2 <= __GNUC__) && (!__STRICT_ANSI__)
#define _C99 1
#endif

#ifdef _SOFT_FLOAT
 /* no FPU hardware support */
 #define _FPP_TYPE _FPP_NONE
#else 
		/* DETERMINE MACHINE TYPE */

 #if defined(i386) || defined(__i386) \
	|| defined(__i386__) || defined(_M_IX86)	/* Pentium */
  #define _FPP_TYPE	_FPP_X86	/* Pentium FPP */
 #elif defined(_MIPS) || defined(_MIPS_) || defined(_M_MRX000) \
    || defined(__MIPS__)		/* MIPS */
  #define _FPP_TYPE	_FPP_MIPS	/* MIPS FPP */
 #elif defined(__ppc__) || defined(_POWER) || defined(_M_PPC) \
    || defined(__PPC__)	/* PowerPC */
  #define _FPP_TYPE	_FPP_PPC	/* PowerPC FPP */
 #elif defined(_ARM_) || defined(__ARM__)	/* ARM */
  #define _FPP_TYPE	_FPP_NONE	/* we don't support ARM FP hardware */
 #elif defined(_SH4_) || defined(__SH__)	/* SH4 */
  #define _FPP_TYPE	_FPP_SH4	/* Hitachi SH4 FPP */
 #else /* system detector */
#error unknown compilation environment
 #endif /* system detector */
#endif

  #define _HAS_POSIX_C_LIB	1	/* use common Unix conventions */

#ifdef __QNX__
/* setting this will stop static and global definitions having a "= 0". */
#define _UNINITALIZED_IS_ZERO		1

/* set this to 1 to compile in atexit cleanup handlers */
#define _TERMINATION_CLEANUP		0

/* set this to 1 to add built-in support POSIX locales */
#define _ADD_POSIX					1

/* Search for locale file here if the LOCFILE envar is not set */
#define _DEFAULT_LOCFILE	"/usr/share/locale/locale.file"

#endif

 #if !defined(_HAS_C9X) && defined(_C99)
  #define _HAS_C9X	1
 #endif /* !defined(_HAS_C9X) etc. */

 #if !defined(__INTEL_COMPILER)
 #define _HAS_C9X_IMAGINARY_TYPE	(_HAS_C9X && __EDG__ \
	&& !defined(__cplusplus))
 #endif 

 #if !defined(_ECPP) && defined(_ABRCPP)
  #define _ECPP
 #endif /* !defined(_ECPP) && defined(_ABRCPP) */

 #if !defined(_IS_EMBEDDED) && defined(_ECPP)
  #define _IS_EMBEDDED	1	/* 1 for Embedded C++ */
 #endif /* _IS_EMBEDDED */

		/* EXCEPTION CONTROL */
#undef _HAS_EXCEPTIONS			/* @@@ qcc defines this when it shouldn't */
 #ifndef _HAS_EXCEPTIONS
  #ifndef _NO_EX	/* don't simplify */
   #define _HAS_EXCEPTIONS	1	/* 1 for try/throw logic */

  #else	/* _NO_EX */
   #define _HAS_EXCEPTIONS	0
  #endif /* _NO_EX */

 #endif /* _HAS_EXCEPTIONS */

		/* NAMING PROPERTIES */
/* #define _STD_LINKAGE	defines C names as extern "C++" */
/* #define _STD_USING	defines C names in namespace std or _Dinkum_std */

 #ifndef _HAS_NAMESPACE
  #ifndef _NO_NS	/* don't simplify */
   #define _HAS_NAMESPACE	1	/* 1 for C++ names in std */

  #else	/* _NO_NS */
   #define _HAS_NAMESPACE	0
  #endif /* _NO_NS */

 #endif /* _HAS_NAMESPACE */

 #if !defined(_STD_USING) && defined(__cplusplus) \
	&& (defined(_C_IN_NS) || 1 < _ALT_NS)
  #define _STD_USING	/* exports C names to global, else reversed */

 #elif defined(_STD_USING) && !defined(__cplusplus)
  #undef _STD_USING	/* define only for C++ */
 #endif /* !defined(_STD_USING) */

 #if !defined(_HAS_STRICT_LINKAGE) \
	&& (__SUNPRO_CC || __EDG__ || !defined(_WIN32_C_LIB))
  #define _HAS_STRICT_LINKAGE	1	/* extern "C" in function type */
 #endif /* !defined(_HAS_STRICT_LINKAGE) */

		/* THREAD AND LOCALE CONTROL */
 #define _MULTI_THREAD	1	/* 0 for no thread locks */

#define _GLOBAL_LOCALE	1	/* 0 for per-thread locales, 1 for shared */
#define _FILE_OP_LOCKS	1	/* 0 for no FILE locks, 1 for atomic */
#define _IOSTREAM_OP_LOCKS	1	/* 0 for no iostream locks, 1 for atomic */

		/* THREAD-LOCAL STORAGE */
#define _COMPILER_TLS	0	/* 1 if compiler supports TLS directly */
#define _TLS_QUAL	/* TLS qualifier, such as __declspec(thread), if any */

 #if !defined(_ADDED_C_LIB)
  #define _ADDED_C_LIB	1	/* include declarations for C extensions */
 #endif /* !defined(_ADDED_C_LIB) */

 #if !defined(_HAS_FIXED_POINT)
  #define _HAS_FIXED_POINT	0	/* enable fixed-point extensions */
 #endif /* !defined(_HAS_FIXED_POINT) */

 #if !defined(_HAS_IMMUTABLE_SETS)
  #define _HAS_IMMUTABLE_SETS	1	/* disallow alterable set elements */
 #endif /* !defined(_HAS_IMMUTABLE_SETS) */

 #if !defined(_HAS_ITERATOR_DEBUGGING) \
	&& (defined(_STL_DB) || defined(_STLP_DEBUG))
  #define _HAS_ITERATOR_DEBUGGING	1	/* enable range checks, etc. */
 #endif /* define _HAS_ITERATOR_DEBUGGING */

 #if !defined(_HAS_STRICT_CONFORMANCE)
  #define _HAS_STRICT_CONFORMANCE	0	/* enable nonconforming extensions */
 #endif /* !defined(_HAS_STRICT_CONFORMANCE) */

 #if !defined(_HAS_TRADITIONAL_IOSTREAMS)
  #define _HAS_TRADITIONAL_IOSTREAMS	1	/* enable old iostreams stuff */
 #endif /* !defined(_HAS_TRADITIONAL_IOSTREAMS) */

 #if !defined(_HAS_TRADITIONAL_ITERATORS)
  #define _HAS_TRADITIONAL_ITERATORS	0	/* don't use pointer iterators */
 #endif /* !defined(_HAS_TRADITIONAL_ITERATORS) */

 #if !defined(_HAS_TRADITIONAL_POS_TYPE)
  #define _HAS_TRADITIONAL_POS_TYPE	0	/* make streampos same as streamoff */
 #endif /* !defined(_HAS_TRADITIONAL_POS_TYPE) */

 #if !defined(_HAS_TRADITIONAL_STL)
  #define _HAS_TRADITIONAL_STL	1	/* enable older STL extensions */
 #endif /* !defined(_HAS_TRADITIONAL_STL) */

 #if !defined(_HAS_TR1)
  #define _HAS_TR1	(!_IS_EMBEDDED)	/* enable TR1 extensions */
 #endif /* !defined(_HAS_TR1) */

#define _HAS_TR1_DECLARATIONS	1	/* always enabled if retained */

 #if !defined(_USE_EXISTING_SYSTEM_NAMES)
  #define _USE_EXISTING_SYSTEM_NAMES	1	/* _Open => open, etc.  */
 #endif /* !defined(_USE_EXISTING_SYSTEM_NAMES) */

 /* FIXME_SUNIL: this is not in the 5.00 yvals.h */
 #if 199901L <= __STDC_VERSION__
 #if defined(__GNUC__) && defined(__cplusplus)
  #define _C99_float_complex	float __complex__
  #define _C99_double_complex	double __complex__
  #define _C99_ldouble_complex	long double __complex__
 #endif /* defined(__GNUC__) && defined(__cplusplus) */
 #endif /* 199901L <= __STDC_VERSION__ */

		/* VC++ COMPILER PARAMETERS */
 #define _CRTIMP
 #define _CDECL

  #define _LONGLONG	long long
  #define _ULONGLONG	unsigned long long
  #define _LLONG_MAX	0x7fffffffffffffffLL
  #define _ULLONG_MAX	0xffffffffffffffffULL

_C_STD_BEGIN
		/* FLOATING-POINT PROPERTIES */
#define _DBIAS	0x3fe	/* IEEE format double and float */
#define _DOFF	4
#define _FBIAS	0x7e
#define _FOFF	7

		/* INTEGER PROPERTIES */
#define _BITS_BYTE	8
#define _C2			1	/* 0 if not 2's complement */
#define _MBMAX		8	/* MB_LEN_MAX */
#define _ILONG		1	/* 0 if 16-bit int */

 #if defined(__s390__) || defined(__CHAR_UNSIGNED__)  \
	|| defined(_CHAR_UNSIGNED)
  #define _CSIGN	0	/* 0 if char is not signed */

 #else /* defined(__s390__) etc */
  #define _CSIGN	1
 #endif /* defined(__s390__) etc */

#define _MAX_EXP_DIG	8	/* for parsing numerics */
#define _MAX_INT_DIG	32
#define _MAX_SIG_DIG	48

		/* stdlib PROPERTIES */
#define _EXFAIL	1	/* EXIT_FAILURE */

		/* stdio PROPERTIES */
#define _FNAMAX	260
#define _FOPMAX	20
#define _TNAMAX	16

  #define _FD_NO(str) ((str)->_Handle)
 #define _FD_VALID(fd)	(0 <= (fd))	/* fd is signed integer */
 #define _FD_INVALID	(-1)
 #define _SYSCH(x)	x

		/* STORAGE ALIGNMENT PROPERTIES */
#define _MEMBND	3U /* eight-byte boundaries (2^^3) */

		/* time PROPERTIES */
#define _CPS	1
#define _TBIAS	((70 * 365LU + 17) * 86400)
_C_STD_END

		/* MULTITHREAD PROPERTIES */

 #if _MULTI_THREAD
_EXTERN_C
void _Locksyslock(int);
void _Unlocksyslock(int);
_END_EXTERN_C

 #else /* _MULTI_THREAD */
  #define _Locksyslock(x)	(void)0
  #define _Unlocksyslock(x)	(void)0
 #endif /* _MULTI_THREAD */

		/* LOCK MACROS */
 #define _LOCK_LOCALE	0
 #define _LOCK_MALLOC	1
 #define _LOCK_STREAM	2
 #define _LOCK_DEBUG	3
 #define _MAX_LOCK		4	/* one more than highest lock number */

 #if _IOSTREAM_OP_LOCKS
  #define _MAYBE_LOCK

 #else /* _IOSTREAM_OP_LOCKS */
  #define _MAYBE_LOCK	\
	if (_Locktype == _LOCK_MALLOC || _Locktype == _LOCK_DEBUG)
 #endif /* _IOSTREAM_OP_LOCKS */

 #ifdef __cplusplus
_STD_BEGIN
extern "C++" {	// in case of _C_AS_CPP
		// CLASS _Lockit
class _Lockit
	{	// lock while object in existence -- MUST NEST
public:

  #if !_MULTI_THREAD
   #define _LOCKIT(x)

	explicit _Lockit()
		{	// do nothing
		}

	explicit _Lockit(int)
		{	// do nothing
		}

	~_Lockit()
		{	// do nothing
		}

  #elif defined(_WIN32_WCE) || defined(_MSC_VER)
   #define _LOCKIT(x)	lockit x

	explicit _Lockit();		// set default lock
	explicit _Lockit(int);	// set the lock
	~_Lockit();	// clear the lock

private:
	int _Locktype;

  #else /* non-Windows multithreading */
   #define _LOCKIT(x)	lockit x

	explicit _Lockit()
		: _Locktype(_LOCK_MALLOC)
		{	// set default lock
		_MAYBE_LOCK
			_Locksyslock(_Locktype);
		}

	explicit _Lockit(int _Type)
		: _Locktype(_Type)
		{	// set the lock
		_MAYBE_LOCK
			_Locksyslock(_Locktype);
		}

	~_Lockit()
		{	// clear the lock
		_MAYBE_LOCK
			_Unlocksyslock(_Locktype);
		}

private:
	int _Locktype;
  #endif /* _MULTI_THREAD */

public:
	_Lockit(const _Lockit&);			// not defined
	_Lockit& operator=(const _Lockit&);	// not defined
	};

class _Mutex
	{	// lock under program control
public:

  #if !_MULTI_THREAD || !_IOSTREAM_OP_LOCKS
    void _Lock()
		{	// do nothing
		}

	void _Unlock()
		{	// do nothing
	}

  #else /* !_MULTI_THREAD || !_IOSTREAM_OP_LOCKS */
	_Mutex();
	~_Mutex();
	void _Lock();
	void _Unlock();

private:
	_Mutex(const _Mutex&);				// not defined
	_Mutex& operator=(const _Mutex&);	// not defined
	void *_Mtx;
  #endif /* !_MULTI_THREAD || !_IOSTREAM_OP_LOCKS */

	};
}	// extern "C++"
_STD_END
 #endif /* __cplusplus */

		/* MISCELLANEOUS MACROS */
#define _ATEXIT_T	void
#define _Atexit(x)	atexit(x)

#define _MAX	max
#define _MIN	min

#ifndef _TEMPLATE_STAT
 #define _TEMPLATE_STAT
#endif /* */

#endif /* _YVALS */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("yvals.h $Rev: 175032 $"); */
