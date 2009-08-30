/* swprintf function */
#include "xwstdio.h"
_STD_BEGIN

void *_Wsprout(void *pa, const wchar_t *buf, size_t request)
	{	/* write to wide string */
	struct _WPargs *p = (struct _WPargs *)pa;
	if (p->max < request)
		{	/* deliver short string */
		wmemcpy(p->s, buf, p->max);
		return (0);
		}
	else
		{	/* deliver full string */
		wmemcpy(p->s, buf, request);
		p->s += request;
		p->max -= request;
		return (pa);
		}
	}
_STD_END

/*
 * Copyright (c) 1994-2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
V3.05:1296 */

__SRCVERSION("xwsprout.c $Rev: 153052 $");
