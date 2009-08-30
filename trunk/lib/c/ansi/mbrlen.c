/* mbrlen function */
#include "xtls.h"
#include "xwchar.h"
_STD_BEGIN

_TLS_DATA_DEF(static, mbstate_t, mbst, {0});

size_t (mbrlen)(const char *_Restrict s, size_t n,
	mbstate_t *_Restrict pst)
	{	/*	determine next multibyte code, restartably */
	return (_Mbtowc(0, s, n, pst ? pst : _TLS_DATA_PTR(mbst)));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mbrlen.c $Rev: 153052 $");
