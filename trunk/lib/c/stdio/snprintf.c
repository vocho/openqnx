/* snprintf function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

typedef struct _Snpstuff
	{	/* package buffer and count */
	char *s;
	size_t size;
	} _Snpstuff;

static void *prout(void *pa, const char *buf, size_t n)
	{	/* write to string */
	_Snpstuff *p = (_Snpstuff *)pa;

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

int (snprintf)(char *_Restrict s, size_t size,
	const char *_Restrict fmt, ...)
	{	/* print formatted to string */
	int ans;
	_Snpstuff x;
	va_list ap;

	va_start(ap, fmt);
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
	va_end(ap);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("snprintf.c $Rev: 153052 $");
