/* swprintf function */
#include "xwstdio.h"
_STD_BEGIN

typedef struct _Swstuff {
	wchar_t *s;
	size_t size;
	} _Swstuff;

static void *prout(void *pa, const wchar_t *buf, size_t n)
	{	/* write to wide string */
	_Swstuff *p = (_Swstuff *)pa;

	if (p->size < n)
		{	/* write only what fits in string */
		n = p->size;
		pa = 0;
		}
	if (0 < n)
		{	/* something to write */
		wmemcpy(p->s, buf, n);
		p->s += n;
		p->size -= n;
		}
	return (pa);
	}

int (swprintf)(wchar_t *_Restrict s, size_t size,
	const wchar_t *_Restrict fmt, ...)
	{	/* print formatted to wide string */
	int ans;
	_Swstuff x;
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
	ans = _WPrintf(&prout, &x, fmt, ap);
	if (x.s != 0)
		*x.s = L'\0';
	va_end(ap);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("swprintf.c $Rev: 153052 $");
