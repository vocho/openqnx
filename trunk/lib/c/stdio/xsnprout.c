/* vsnprintf function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

void *_Snprout(void *pa, const char *buf, size_t n)
	{	/* write to string */
	struct _Pargs *p = (struct _Pargs *)pa;
	if (p->max <= n)
		{	/* deliver short string */
#ifdef __QNX__
		/* We need to always return the number of bytes which we
		   would have put into the buffer.  This means that we
		   will fill the buffer and from that point on only "pretend"
		   to actually copy the data.
		*/
		if(p->max > 0) 
			{
			memcpy(p->s, buf, p->max);
			p->s += p->max;
			p->max -= p->max;
			}
		return (pa);
#else
		memcpy(p->s, buf, p->max);
		return (0);
#endif
		}
	else
		{	/* deliver full string */
		memcpy(p->s, buf, n);
		p->s += n;
		p->max -= n;
		return (pa);
		}
	}
_STD_END

/*
 * Copyright (c) 1994-2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

__SRCVERSION("xsnprout.c $Rev: 153052 $");
