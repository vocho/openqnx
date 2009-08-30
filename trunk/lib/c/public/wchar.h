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
 *  wchar.h    Amendment 1 wide char definitions
 *
 * Copyright (c) 1994-2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */



#ifndef _WCHAR_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _WCHAR_H_INCLUDED
#endif

#ifndef _WCHAR_H_DECLARED
#define _WCHAR_H_DECLARED

_C_STD_BEGIN

#ifndef NULL
 #define NULL 			_NULL
#endif

#ifndef _Mbstinit
 #define _Mbstinit(x)	_Mbstatet x = {0, 0}
#endif /* _Mbstinit */

#define WCHAR_MIN		_WCMIN
#define WCHAR_MAX		_WCMAX
#define WEOF			((wint_t)(-1))

#if defined(__MBSTATE_T)
typedef __MBSTATE_T		mbstate_t;
#undef __MBSTATE_T
#endif

#if defined(__WCHAR_T)
typedef __WCHAR_T		wchar_t;
#undef __WCHAR_T
#endif

#ifndef _WINTT
typedef long _Wintt;
#define _WINTT
typedef _Wintt wint_t;
#endif /* _WINTT */

#if defined(__SIZE_T)
typedef __SIZE_T		size_t;
#undef __SIZE_T
#endif

_C_STD_END
struct _Filet;
_C_STD_BEGIN
struct tm;

_C_LIB_DECL
		/* stdio DECLARATIONS */
wint_t fgetwc(struct _Filet *);
wchar_t *fgetws(wchar_t *, int, struct _Filet *);
wint_t fputwc(wchar_t, struct _Filet *);
int fputws(const wchar_t *, struct _Filet *);
int fwide(struct _Filet *, int);
int fwprintf(struct _Filet *, const wchar_t *, ...);
int fwscanf(struct _Filet *, const wchar_t *, ...);
int vfwscanf(struct _Filet *_Restrict, const wchar_t *_Restrict, __NTO_va_list);
int vswscanf(const wchar_t *_Restrict, const wchar_t *_Restrict, __NTO_va_list);
int vwscanf(const wchar_t *_Restrict, __NTO_va_list);
wint_t getwc(struct _Filet *);
wint_t getwchar(void);
wint_t putwc(wchar_t, struct _Filet *);
wint_t putwchar(wchar_t);
int swprintf(wchar_t *, size_t, const wchar_t *, ...);
int swscanf(const wchar_t *, const wchar_t *, ...);
wint_t ungetwc(wint_t, struct _Filet *);
int vfwprintf(struct _Filet *, const wchar_t *, __NTO_va_list);
int vswprintf(wchar_t *, size_t, const wchar_t *, __NTO_va_list);
int vwprintf(const wchar_t *, __NTO_va_list);
int wprintf(const wchar_t *, ...);
int wscanf(const wchar_t *, ...);

		/* stdlib DECLARATIONS */
size_t mbrlen(const char *, size_t, mbstate_t *);
size_t mbrtowc(wchar_t *, const char *, size_t,
	mbstate_t *);
size_t mbsrtowcs(wchar_t *, const char **, size_t,
	mbstate_t *);
int mbsinit(const mbstate_t *);
size_t wcrtomb(char *, wchar_t, mbstate_t *);
size_t wcsrtombs(char *, const wchar_t **, size_t,
	mbstate_t *);
long wcstol(const wchar_t *, wchar_t **, int);
_Longlong wcstoll(const wchar_t *, wchar_t **, int);

		/* string DECLARATIONS */
wchar_t *wcscat(wchar_t *, const wchar_t *);
int wcscmp(const wchar_t *, const wchar_t *);
int wcscoll(const wchar_t *, const wchar_t *);
wchar_t *wcscpy(wchar_t *, const wchar_t *);
size_t wcscspn(const wchar_t *, const wchar_t *);
size_t wcslen(const wchar_t *);
wchar_t *wcsncat(wchar_t *, const wchar_t *, size_t);
int wcsncmp(const wchar_t *, const wchar_t *, size_t);
wchar_t *wcsncpy(wchar_t *, const wchar_t *, size_t);
size_t wcsspn(const wchar_t *, const wchar_t *);
wchar_t *wcstok(wchar_t *, const wchar_t *,
	wchar_t **);
size_t wcsxfrm(wchar_t *, const wchar_t *, size_t);
int wmemcmp(const wchar_t *, const wchar_t *, size_t);
wchar_t *wmemcpy(wchar_t *, const wchar_t *, size_t);
wchar_t *wmemmove(wchar_t *, const wchar_t *, size_t);
wchar_t *wmemset(wchar_t *, wchar_t, size_t);

		/* time DECLARATIONS */
size_t wcsftime(wchar_t *, size_t, const wchar_t *,
	const struct tm *);
wint_t _Btowc(int);
int _Wctob(wint_t);
double _WStod(const wchar_t *, wchar_t **, long);
float _WStof(const wchar_t *, wchar_t **, long);
long double _WStold(const wchar_t *, wchar_t **, long);
unsigned long _WStoul(const wchar_t *, wchar_t **, int);
_ULonglong _WStoull(const wchar_t *, wchar_t **, int);
_END_C_LIB_DECL

#ifdef __cplusplus
		/* INLINES AND OVERLOADS, FOR C++ */
#define _WConst_return const

_C_LIB_DECL
const wchar_t *wmemchr(const wchar_t *, wchar_t, size_t);
const wchar_t *wcschr(const wchar_t *, wchar_t);
const wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
const wchar_t *wcsrchr(const wchar_t *, wchar_t);
const wchar_t *wcsstr(const wchar_t *, const wchar_t *);
_END_C_LIB_DECL

inline wchar_t *wmemchr(wchar_t *_S, wchar_t _C, size_t _N)
	{	/* call with const first argument */
	union { wchar_t *_Out; const wchar_t * _In; } _Result;
	return (_Result._In = wmemchr((const wchar_t *)_S, _C, _N)), _Result._Out; }

inline wchar_t *wcschr(wchar_t *_S, wchar_t _C)
	{	/* call with const first argument */
	union { wchar_t *_Out; const wchar_t * _In; } _Result;
	return (_Result._In = wcschr((const wchar_t *)_S, _C)), _Result._Out; }

inline wchar_t *wcspbrk(wchar_t *_S, const wchar_t *_P)
	{	/* call with const first argument */
	union { wchar_t *_Out; const wchar_t * _In; } _Result;
	return (_Result._In = wcspbrk((const wchar_t *)_S, _P)), _Result._Out; }

inline wchar_t *wcsrchr(wchar_t *_S, wchar_t _C)
	{	/* call with const first argument */
	union { wchar_t *_Out; const wchar_t * _In; } _Result;
	return (_Result._In = wcsrchr((const wchar_t *)_S, _C)), _Result._Out; }

inline wchar_t *wcsstr(wchar_t *_S, const wchar_t *_P)
	{	/* call with const first argument */
	union { wchar_t *_Out; const wchar_t * _In; } _Result;
	return (_Result._In = wcsstr((const wchar_t *)_S, _P)), _Result._Out; }

inline wint_t btowc(int _C)
	{	/* convert single byte to wide character */
	return (_Btowc(_C)); }

inline double wcstod(const wchar_t *_S,
	wchar_t **_Endptr)
	{	/* convert wide string to double */
	return (_WStod(_S, _Endptr, 0)); }

inline unsigned long wcstoul(const wchar_t *_S,
	wchar_t **_Endptr, int _Base)
	{	/* convert wide string to unsigned long */
	return (_WStoul(_S, _Endptr, _Base)); }

inline _ULonglong wcstoull(const wchar_t *_S,
	wchar_t **_Endptr, int _Base)
	{	/* convert wide string to unsigned long long */
	return (_WStoull(_S, _Endptr, _Base)); }

inline int wctob(wint_t _Wc)
	{	/* convert wide character to single byte */
	return (_Wctob(_Wc)); }

#else /* __cplusplus */
#define _WConst_return

_C_LIB_DECL
wchar_t *wmemchr(const wchar_t *, wchar_t, size_t);
wchar_t *wcschr(const wchar_t *, wchar_t);
wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
wchar_t *wcsrchr(const wchar_t *, wchar_t);
wchar_t *wcsstr(const wchar_t *, const wchar_t *);
wint_t btowc(int);
double wcstod(const wchar_t *, wchar_t **);
float wcstof(const wchar_t *, wchar_t **);
long double wcstold(const wchar_t *, wchar_t **);
unsigned long wcstoul(const wchar_t *, wchar_t **, int);
_ULonglong wcstoull(const wchar_t *, wchar_t **, int);
int wctob(wint_t);
_END_C_LIB_DECL

#define btowc(c)	_Btowc(c)
#define wcstod(s, endptr)	_WStod(s, endptr, 0)
#define wcstof(s, endptr)	_WStof(s, endptr, 0)
#define wcstold(s, endptr)	_WStold(s, endptr, 0)
#define wcstoul(s, endptr, base)	_WStoul(s, endptr, base)
#define wcstoull(s, endptr, base)	_WStoull(s, endptr, base)
#define wctob(wc)	_Wctob(wc)
#endif /* __cplusplus */
_C_STD_END

#endif /* _WCHAR */

#ifdef _STD_USING
using std::mbstate_t; using std::size_t; using std::tm; using std::wint_t;
using std::btowc; using std::fgetwc; using std::fgetws; using std::fputwc;
using std::fputws; using std::fwide; using std::fwprintf;
using std::fwscanf; using std::getwc; using std::getwchar;
using std::mbrlen; using std::mbrtowc; using std::mbsrtowcs;
using std::mbsinit; using std::putwc; using std::putwchar;
using std::swprintf; using std::swscanf; using std::ungetwc;
using std::vfwprintf; using std::vswprintf; using std::vwprintf;
using std::wcrtomb; using std::wprintf; using std::wscanf;
using std::wcsrtombs; using std::wcstol; using std::wcstoll; using std::wcscat;
using std::wcschr; using std::wcscmp; using std::wcscoll;
using std::wcscpy; using std::wcscspn; using std::wcslen;
using std::wcsncat; using std::wcsncmp; using std::wcsncpy;
using std::wcspbrk; using std::wcsrchr; using std::wcsspn;
using std::wcstod; using std::wcstoul; using std::wcstoull; using std::wcsstr;
using std::wcstok; using std::wcsxfrm; using std::wctob;
using std::wmemchr; using std::wmemcmp; using std::wmemcpy;
using std::wmemmove; using std::wmemset; using std::wcsftime;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("wchar.h $Rev: 171322 $"); */
