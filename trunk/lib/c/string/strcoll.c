/* strcoll function */
#include "xstrxfrm.h"
 #include "xmtloc.h"
_STD_BEGIN

		/* type definitions */
typedef struct Storage {
	char buf[32];
	const char *sout;
	Xfrm xstate;
	} Storage;

static size_t getxfrm(Storage *p, _Statab *pcostate)
	{	/* get transformed chars */
	p->sout = (const char *)p->buf;
	return (_CStrxfrm(p->buf, sizeof (p->buf), &p->xstate, pcostate));
	}

int _Strcollx(const char *s1, const char *s2, _Statab *pcostate)
	{	/* compare s1[], s2[] using given locale-dependent rule */
#ifdef __QNX__
	static const Storage initial;
#else
	static const Storage initial = {0};
#endif
	Storage st1 = initial;
	Storage st2 = initial;
	size_t n1, n2;

	st1.xstate.sbegin = (const unsigned char *)s1;
	st2.xstate.sbegin = (const unsigned char *)s2;
	for (n1 = n2 = 0; ; )
		{	/* compare transformed chars */
		int ans;
		size_t n;

		if (n1 == 0)
			n1 = getxfrm(&st1, pcostate);
		if (n2 == 0)
			n2 = getxfrm(&st2, pcostate);
		n = n1 < n2 ? n1 : n2;
		if (n == 0)
			return (n1 == n2 ? 0 : 0 < n2 ? -1 : +1);
		else if ((ans = memcmp(st1.sout, st2.sout, n)) != 0
			|| n1 == n2 && st1.sout[n - 1] == '\0')
			return (ans);
		st1.sout += n, n1 -= n;
		st2.sout += n, n2 -= n;
		}
	}

int (strcoll)(const char *s1, const char *s2)
	{	/* compare s1[], s2[] using global locale-dependent rule */
	return (_Strcollx(s1, s2, _TLS_DATA_PTR(_Costate)));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strcoll.c $Rev: 153052 $");
