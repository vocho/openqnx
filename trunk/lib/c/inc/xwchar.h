/* xwchar.h internal header */
#ifndef _XWCHAR
#define _XWCHAR
#include <wchar.h>
#include <wctype.h>
#include <xstate.h>
_C_STD_BEGIN

		/* DECLARATIONS */
_C_LIB_DECL
int _Mbtowc(wchar_t *, const char *, size_t, mbstate_t *);
size_t _Wcsftime(wchar_t *, size_t, const char *, size_t,
	const struct tm *);
int _Wctomb(char *, wchar_t, mbstate_t *);
long double _WStold(const wchar_t *, wchar_t **, long);
_Longlong _WStoll(const wchar_t *, wchar_t **, int);
unsigned long _WStoul(const wchar_t *, wchar_t **, int);
_ULonglong _WStoull(const wchar_t *, wchar_t **, int);

int _Mbtowcx(wchar_t *, const char *, size_t, mbstate_t *,
	_Statab *);
int _Wctombx(char *, wchar_t, mbstate_t *,
	_Statab *, _Statab *);

_Statab *_Getpmbstate(void);
_Statab *_Getpwcstate(void);
_Statab *_Getpcostate(void);
_Statab *_Getpwcostate(void);
_END_C_LIB_DECL
_C_STD_END
#endif /* _XWCHAR */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xwchar.h $Rev: 153052 $"); */
