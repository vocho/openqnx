/* vsprintf function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

typedef struct _Vsnpstuff
	{	/* package buffer and count */
	char *s;
	size_t size;
	} _Vsnpstuff;

static void *prout(void *pa, const char *buf, size_t n)
	{	/* write to string */
	_Vsnpstuff *p = (_Vsnpstuff *)pa;

	if (p->size < n)
		n = p->size;	/* write only what fits in string */
	if (0 < n)
		{	/* something to write */
		memcpy(p->s, buf, n);
		p->s += n;
		p->size -= n;
		}
	return (pa);
	}

int (vsnprintf)(char *_Restrict s, size_t size,
	const char *_Restrict fmt, va_list ap)
	{	/* print formatted to string from arg list */
	int ans;
	_Vsnpstuff x;

	if (size == 0)
		{	/* write nothing */
		x.s = 0;
		x.size = 0;
		}
	else
		{	/* set up buffer */
		x.s = s;
		x.size = size - 1;
		}
	ans = _Printf(&prout, &x, fmt, ap);
	if (x.s != 0)
		*x.s = '\0';
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("vsnprint.c $Rev: 153052 $");
