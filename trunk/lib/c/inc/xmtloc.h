/* xmtloc.h internal header */
#ifndef _XMTLOC
#define _XMTLOC
#include <ctype.h>
#include <xstate.h>
#include <xtinfo.h>
#include <xtls.h>
#include <xwctype.h>

_C_STD_BEGIN

_C_LIB_DECL
_TLS_DATA_DECL(_Statab, _Costate);
_TLS_DATA_DECL(_Statab, _WCostate);
_TLS_DATA_DECL(_Statab, _Mbstate);
_TLS_DATA_DECL(_Statab, _Wcstate);
_TLS_DATA_DECL(_Ctype_t, _Ctype);
_TLS_DATA_DECL(_PWctab, _Wctrans);
_TLS_DATA_DECL(_PWctab, _Wctype);
_TLS_DATA_DECL(_Ctype_t, _Tolotab);
_TLS_DATA_DECL(_Ctype_t, _Touptab);
_TLS_DATA_DECL(char, _Mbcurmax);
_TLS_DATA_DECL(struct lconv, _Locale);
_TLS_DATA_DECL(_Tinfo, _Times);
_END_C_LIB_DECL

_C_STD_END
#endif /* _XMTLOC */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xmtloc.h $Rev: 153052 $"); */
