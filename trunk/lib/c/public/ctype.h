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





/* *  ctype.h     Character Handling
 *
 * Copyright (c) 1994-2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

#ifndef _CTYPE_H_INCLUDED

#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _CTYPE_H_INCLUDED
#endif

#ifndef _CTYPE_H_DECLARED
#define _CTYPE_H_DECLARED

_C_STD_BEGIN

		/* _Ctype code bits */
#define _XB		0x400 /* extra blank */
#define _XA		0x200 /* extra alphabetic */
#define _XS		0x100 /* extra space */
#define _BB		0x80 /* BEL, BS, etc. */
#define _CN		0x40 /* CR, FF, HT, NL, VT */
#define _DI		0x20 /* '0'-'9' */
#define _LO		0x10 /* 'a'-'z' */
#define _PU		0x08 /* punctuation */
#define _SP		0x04 /* space */
#define _UP		0x02 /* 'A'-'Z' */
#define _XD		0x01 /* '0'-'9', 'A'-'F', 'a'-'f' */

_C_LIB_DECL
typedef const short *_Ctype_t;

_Ctype_t _Getpctype(void);
_Ctype_t _Getptolower(void);
_Ctype_t _Getptoupper(void);

extern _Ctype_t _Ctype;
extern _Ctype_t _Tolotab;
extern _Ctype_t _Touptab;

  #define _Getpctype()		_CSTD _Ctype
  #define _Getptolower()	_CSTD _Tolotab
  #define _Getptoupper()	_CSTD _Touptab

#define _Getchrtype(x)	_Getpctype()[(int)(x)]
#define _CTolower(x)	_Getptolower()[(int)(x)]
#define _CToupper(x)	_Getptoupper()[(int)(x)]
_END_C_LIB_DECL

 #if defined(__cplusplus) && !defined(_NO_CPP_INLINES)
		// INLINES, for C++
_C_LIB_DECL
inline int isalnum(int _Ch)
	{	// check alnum bits
	return (_Getchrtype(_Ch) & (_DI|_LO|_UP|_XA));
	}

inline int isalpha(int _Ch)
	{	// check alpha bits
	return (_Getchrtype(_Ch) & (_LO|_UP|_XA));
	}

inline int iscntrl(int _Ch)
	{	// check cntrl bits
	return (_Getchrtype(_Ch) & _BB);
	}

inline int isdigit(int _Ch)
	{	// check digit bits
	return (_Getchrtype(_Ch) & _DI);
	}

inline int isgraph(int _Ch)
	{	// check graph bits
	return (_Getchrtype(_Ch) & (_DI|_LO|_PU|_UP|_XA));
	}

inline int islower(int _Ch)
	{	// check alnum bits
	return (_Getchrtype(_Ch) & _LO);
	}

inline int isprint(int _Ch)
	{	// check print bits
	return (_Getchrtype(_Ch) & (_DI|_LO|_PU|_SP|_UP|_XA));
	}

inline int ispunct(int _Ch)
	{	// check punct bits
	return (_Getchrtype(_Ch) & _PU);
	}

inline int _Isspace(int _Ch)
	{	// check space bits
	return (_Getchrtype(_Ch) & (_CN|_SP|_XS));
	}

inline int isspace(int _Ch)
	{	// check space bits
	return (_Isspace(_Ch));
	}

inline int isupper(int _Ch)
	{	// check upper bits
	return (_Getchrtype(_Ch) & _UP);
	}

inline int isxdigit(int _Ch)
	{	// check xdigit bits
	return (_Getchrtype(_Ch) & _XD);
	}

 #if _HAS_C9X
inline int isblank(int _Ch)
	{	// check blank bits
	return (_Getchrtype(_Ch) & (_SP|_XB));
	}
 #endif /* _IS_C9X */

inline int tolower(int _Ch)
	{	// convert to lower case
	return (_CTolower(_Ch));
	}

inline int toupper(int _Ch)
	{	// convert to upper case
	return (_CToupper(_Ch));
	}
_END_C_LIB_DECL

#ifdef __EXT_XOPEN_EX

inline int _tolower(int _Ch) {	/* convert to lower case */
	return (_CSTD _Tolotab[_Ch]); }

inline int _toupper(int _Ch) {	/* convert to upper case */
	return (_CSTD _Touptab[_Ch]); }

inline int isascii(int _Ch) {	/* check ascii */
	return ((unsigned)(_Ch) <= 0x7f); }

inline int toascii(int _Ch) {	/* convert to ascii */
	return ((_Ch) & 0x7f); }

#endif

inline int __isascii(int _Ch) {	/* check ascii */
	return ((unsigned)(_Ch) <= 0x7f); }

inline int __toascii(int _Ch) {	/* convert to ascii */
	return ((int)(_Ch) & 0x7f); }

inline int __iscsymf(int _Ch) {	/* check first C symbol */
	return (_CSTD isalpha(_Ch)||(_Ch)=='_'); }

inline int __iscsym(int _Ch) {	/* check next C symbol */
	return (_CSTD isalnum(_Ch)||(_Ch)=='_'); }

 #else /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */
		/* declarations and macro overrides, for C */
_C_LIB_DECL

int isalnum(int);
int isalpha(int);
int iscntrl(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int ispunct(int);
int isspace(int);
int isupper(int);
int isxdigit(int);
int tolower(int);
int toupper(int);
 #if _HAS_C9X
int isblank(int);
 #endif /* _IS_C9X */

#ifdef __EXT_XOPEN_EX
extern int _tolower(int), _toupper(int);
extern int isascii(int), toascii(int);
#endif
extern int __isascii(int), __toascii(int);
extern int __iscsymf(int), __iscsym(int);

_END_C_LIB_DECL

  #define isalnum(c)	(_Getchrtype(c) & (_DI|_LO|_UP|_XA))
  #define isalpha(c)	(_Getchrtype(c) & (_LO|_UP|_XA))
  #define iscntrl(c)	(_Getchrtype(c) & _BB)
  #define isdigit(c)	(_Getchrtype(c) & _DI)
  #define isgraph(c)	(_Getchrtype(c) & (_DI|_LO|_PU|_UP|_XA))
  #define islower(c)	(_Getchrtype(c) & _LO)
  #define isprint(c)	(_Getchrtype(c) & (_DI|_LO|_PU|_SP|_UP|_XA))
  #define ispunct(c)	(_Getchrtype(c) & _PU)
  #define isspace(c)	(_Getchrtype(c) & (_CN|_SP|_XS))
  #define isupper(c)	(_Getchrtype(c) & _UP)
  #define isxdigit(c)	(_Getchrtype(c) & _XD)
 #if _HAS_C9X
  #define isblank(c)	(_Getchrtype(c) & (_SP|_XB))
 #endif /* _IS_C9X */

   #define tolower(c)	_CTolower(c)
   #define toupper(c)	_CToupper(c)

#ifdef __EXT_XOPEN_EX
#define _tolower(__c)	_CTolower(__c)
#define _toupper(__c)	_CToupper(__c)
#define isascii(__c)	((unsigned)(__c) <= 0x7f)
#define toascii(__c)	((__c) & 0x7f)
#endif
#define __iscsymf(__c)	(isalpha(__c)||((__c)=='_'))
#define __iscsym(__c)	(isalnum(__c)||((__c)=='_'))
#define __isascii(__c)	((unsigned)(__c) <= 0x7f)
#define __toascii(__c)	((__c) & 0x7f)

 #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

_C_STD_END
#endif /* _CTYPE */

#ifdef _STD_USING
using _CSTD isalnum; using _CSTD isalpha; using _CSTD iscntrl;
using _CSTD isdigit; using _CSTD isgraph; using _CSTD islower;
using _CSTD isprint; using _CSTD ispunct; using _CSTD isspace;
using _CSTD isupper; using _CSTD isxdigit; using _CSTD tolower;
using _CSTD toupper; using _CSTD isblank;
#ifdef __EXT_XOPEN_EX
using _CSTD toascii; using _CSTD isascii;
#endif

 #if _HAS_C9X
using _CSTD isblank;
 #endif /* _IS_C9X */

#endif /* _STD_USING */

#endif

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("ctype.h $Rev: 153052 $"); */
