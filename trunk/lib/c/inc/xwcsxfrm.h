/* xwcsxfrm.h internal header */
#ifndef _XWCSXFRM
#define _XWCSXFRM
#include <xstate.h>
#include <xwchar.h>
_C_STD_BEGIN

		/* declarations */
_C_LIB_DECL
int _Wcscollx(const wchar_t *,
	const wchar_t *, _Statab *);
size_t _Wcsxfrmx(wchar_t *,
	const wchar_t *, size_t, _Statab *);

size_t _CWcsxfrm(wchar_t *, const wchar_t **, size_t,
	mbstate_t *, _Statab *);
_END_C_LIB_DECL
_C_STD_END
#endif /* _XWCSXFRM */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xwcsxfrm.h $Rev: 153052 $"); */
