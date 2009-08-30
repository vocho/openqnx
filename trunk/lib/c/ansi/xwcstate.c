/* _Wcstate table */
#include "xlocale.h"
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(_IMPLICIT_EXTERN, _Statab, _Wcstate,
	{{0}});	/* 0: 1-to-1 inline */

_Statab *_Getpwcstate()
	{	/* get pointer to _Wcstate */
	return (_TLS_DATA_PTR(_Wcstate));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwcstate.c $Rev: 153052 $");
