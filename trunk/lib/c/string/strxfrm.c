/* strxfrm function */
#include "xstrxfrm.h"
#include "xmtloc.h"
_STD_BEGIN

size_t _Strxfrmx(char *s1, const char *s2, size_t n, _Statab *pcostate)
	{	/* transform s2[] to s1[] using given locale-dependent rule */
#ifdef __QNX__
	static const Xfrm initial;
#else
	static const Xfrm initial = {0};
#endif
	Xfrm xstate = initial;
	size_t nx = 0;

	xstate.sbegin = (const unsigned char *)s2;
	while (nx < n)
		{	/* translate and deliver */
		size_t i = _CStrxfrm((char *)s1, n - nx, &xstate, pcostate);

		s1 += i, nx += i;
		if (0 < i && s1[-1] == '\0')
			return (nx - 1);
		}
	for (; ; )
		{	/* translate and count */
		char buf[32];
		size_t i = _CStrxfrm(buf, sizeof (buf), &xstate, pcostate);

		nx += i;
		if (0 < i && buf[i - 1] == '\0')
			return (nx - 1);
		}
	}

size_t (strxfrm)(char *_Restrict s1, const char *_Restrict s2,
	size_t n)
	{	/* transform s2[] to s1[] using global locale-dependent rule */
	return (_Strxfrmx(s1, s2, n, _TLS_DATA_PTR(_Costate)));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strxfrm.c $Rev: 153052 $");
