/* _WCostate table */
#include "xlocale.h"
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(_IMPLICIT_EXTERN, _Statab, _WCostate,
	{{0}});	/* 0: 1-to-1 inline */

_Statab *_Getpwcostate()
	{	/* get pointer to _WCostate */
	return (_TLS_DATA_PTR(_WCostate));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwcostate.c $Rev: 153052 $");
