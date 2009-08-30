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



/*
 *  locale.h
 *

 *
 * Copyright (c) 1994-1999 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */
#ifndef _LOCALE_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _LOCALE_H_INCLUDED
#endif

#ifndef _LOCALE_H_DECLARED
#define _LOCALE_H_DECLARED

_C_STD_BEGIN

		/* macros */
#ifndef NULL
 #define NULL	_NULL
#endif

		/* locale category indexes */
#define _LC_COLLATE		1
#define _LC_CTYPE		2
#define _LC_MONETARY	3
#define _LC_NUMERIC		4
#define _LC_TIME		5
#define _LC_MESSAGES	6
	/* ADD YOURS HERE, THEN UPDATE _NCAT */
#define _NCAT			7	/* one more than last index */

		/* locale category masks */
#define _CATMASK(n)	((1U << (n)) >> 1)
#define _M_COLLATE	_CATMASK(_LC_COLLATE)
#define _M_CTYPE	_CATMASK(_LC_CTYPE)
#define _M_MONETARY	_CATMASK(_LC_MONETARY)
#define _M_NUMERIC	_CATMASK(_LC_NUMERIC)
#define _M_TIME		_CATMASK(_LC_TIME)
#define _M_MESSAGES	_CATMASK(_LC_MESSAGES)
#define _M_ALL		(_CATMASK(_NCAT) - 1)

		/* locale category handles */
#define LC_COLLATE	_CATMASK(_LC_COLLATE)
#define LC_CTYPE	_CATMASK(_LC_CTYPE)
#define LC_MONETARY	_CATMASK(_LC_MONETARY)
#define LC_NUMERIC	_CATMASK(_LC_NUMERIC)
#define LC_TIME		_CATMASK(_LC_TIME)
#define LC_MESSAGES	_CATMASK(_LC_MESSAGES)
#define LC_ALL		(_CATMASK(_NCAT) - 1)

		/* type definitions */
struct lconv
	{	/* locale-specific information */
		/* controlled by LC_MONETARY */
	char *currency_symbol;
	char *int_curr_symbol;
	char *mon_decimal_point;
	char *mon_grouping;
	char *mon_thousands_sep;
	char *negative_sign;
	char *positive_sign;
	char frac_digits;
	char int_frac_digits;
	char n_cs_precedes;
	char n_sep_by_space;
	char n_sign_posn;
	char p_cs_precedes;
	char p_sep_by_space;
	char p_sign_posn;

	char int_n_cs_precedes;	/* Added with C99 */
	char int_n_sep_by_space;	/* Added with C99 */
	char int_n_sign_posn;		/* Added with C99 */
	char int_p_cs_precedes;	/* Added with C99 */
	char int_p_sep_by_space;	/* Added with C99 */
	char int_p_sign_posn;		/* Added with C99 */

		/* controlled by LC_NUMERIC */
	char *decimal_point;
	char *grouping;
	char *thousands_sep;
	char *_Frac_grouping;
	char *_Frac_sep;
	char *_False;
	char *_True;

		/* controlled by LC_MESSAGES */
	char *_No;
	char *_Yes;
	char *_Nostr;
	char *_Yesstr;
	char *_Reserved[8];
	};

struct _Linfo;

		/* declarations */
_C_LIB_DECL
struct lconv *localeconv(void);
char *setlocale(int, const char *);
extern struct lconv _Locale;
_END_C_LIB_DECL

#ifdef __cplusplus
		/* inlines, for C++ */
inline struct lconv *localeconv(void)
	{	/* return pointer to current locale */
	return (&_Locale); }
#else
		/* macro overrides, for C */
#define localeconv()	(&_CSTD _Locale)
#endif /* __cplusplus */
_C_STD_END

#endif /* _LOCALE */

#ifdef _STD_USING
using _CSTD lconv; using _CSTD localeconv; using _CSTD setlocale;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("locale.h $Rev: 153052 $"); */
