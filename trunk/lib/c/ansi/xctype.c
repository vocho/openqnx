/* _Ctype conversion table -- ASCII version */
#include <ctype.h>
#include "xtls.h"
 #include <limits.h>
 #include <stdio.h>

 #if EOF != -1 || UCHAR_MAX != 255
  #error WRONG CTYPE TABLE
 #endif /* EOF != -1 || UCHAR_MAX != 256 */

_STD_BEGIN

		/* macros */
 #define XBB (_BB | _CN)
 #define XBL (XBB | _XB)
 #define XDI (_DI | _XD)
 #define XLO (_LO | _XD)
 #define XUP (_UP | _XD)

		/* static data */
static const short ctyp_tab[257] = {0, /* EOF */
 _BB, _BB, _BB, _BB, _BB, _BB, _BB, _BB,
 _BB, XBL, XBB, XBB, XBB, XBB, _BB, _BB,
 _BB, _BB, _BB, _BB, _BB, _BB, _BB, _BB,
 _BB, _BB, _BB, _BB, _BB, _BB, _BB, _BB,
 _SP, _PU, _PU, _PU, _PU, _PU, _PU, _PU,
 _PU, _PU, _PU, _PU, _PU, _PU, _PU, _PU,
 XDI, XDI, XDI, XDI, XDI, XDI, XDI, XDI,
 XDI, XDI, _PU, _PU, _PU, _PU, _PU, _PU,
 _PU, XUP, XUP, XUP, XUP, XUP, XUP, _UP,
 _UP, _UP, _UP, _UP, _UP, _UP, _UP, _UP,
 _UP, _UP, _UP, _UP, _UP, _UP, _UP, _UP,
 _UP, _UP, _UP, _PU, _PU, _PU, _PU, _PU,
 _PU, XLO, XLO, XLO, XLO, XLO, XLO, _LO,
 _LO, _LO, _LO, _LO, _LO, _LO, _LO, _LO,
 _LO, _LO, _LO, _LO, _LO, _LO, _LO, _LO,
 _LO, _LO, _LO, _PU, _PU, _PU, _PU, _BB,
 };	/* rest all match nothing */

_TLS_DATA_DEF(_IMPLICIT_EXTERN, _Ctype_t, _Ctype, &ctyp_tab[1]);

_Ctype_t (_Getpctype)(void)
	{	/* get _Ctype table pointer */
	return (*_TLS_DATA_PTR(_Ctype));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xctype.c $Rev: 153052 $");
