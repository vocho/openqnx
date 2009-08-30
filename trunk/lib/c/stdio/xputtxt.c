/* _Puttxt function */
#include "xstdio.h"
_STD_BEGIN

		/* macros */
#define MAX_PAD	(sizeof (spaces) - 1)

#define PAD(px, s, n, ns)	\
	if (0 < (n)) \
		{int i, j = (n); \
		for (; 0 < j; j -= i) \
			{i = (ns) < j ? (ns) : j; \
			{PUT(px, s, i)} \
			} \
		}	/* pad n elements from s[ns] */

#define PUT(px, s, n)	\
	if (0 < (n)) \
		if (((px)->arg = (*(px)->pfn)((px)->arg, s, n)) != 0) \
			(px)->nchar += (n); \
		else \
			return (EOF);	/* put n elements from s */

		/* static data */
static const char spaces[] =
	"                                ";
static const char zeroes[] =
	"00000000000000000000000000000000";

int _Puttxt(_Pft *px, const char *ac)
	{	/* print generated text plus padding */
	int width = (((((px->width - px->n0) - px->nz0) - px->n1)
		- px->nz1) - px->n2) - px->nz2;

	if (!(px->flags & _FMI))
		{PAD(px, &spaces[0], width, MAX_PAD)}
	{PUT(px, ac, px->n0)}
	{PAD(px, &zeroes[0], px->nz0, MAX_PAD)}
	{PUT(px, px->s, px->n1)}
	{PAD(px, &zeroes[0], px->nz1, MAX_PAD)}
	{PUT(px, px->s + px->n1, px->n2)}
	{PAD(px, &zeroes[0], px->nz2, MAX_PAD)}
	if (px->flags & _FMI)
		{PAD(px, &spaces[0], width, MAX_PAD)}
	return (0);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xputtxt.c $Rev: 153052 $");
