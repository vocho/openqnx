/* xstrxfrm.h internal header */
#ifndef _XSTRXFRM
#define _XSTRXFRM
#include <string.h>
#include <xstate.h>
_C_STD_BEGIN

		/* TYPES */
typedef struct Xfrm {	/* storage for transformations */
	const unsigned char *sbegin;
	const unsigned char *sin;
	const unsigned char *send;
	long weight;
	unsigned short phase, state, wc;
	} Xfrm;

		/* DECLARATIONS */
_C_LIB_DECL
int _Strcollx(const char *, const char *, _Statab *);
size_t _Strxfrmx(char *, const char *, size_t, _Statab *);

size_t _CStrxfrm(char *, size_t, Xfrm *, _Statab *);
_END_C_LIB_DECL
_C_STD_END
#endif /* _XSTRXFRM */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xstrxfrm.h $Rev: 153052 $"); */
