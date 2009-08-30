/* _WPuttxt function */
#include "xwstdio.h"
_STD_BEGIN

		/* macros */
#define WMAX_PAD	((sizeof (spaces) - 1) / sizeof (wchar_t))

#define WPAD(px, s, n, ns)	\
	if (0 < (n)) \
		{int i, j = (n); \
		for (; 0 < j; j -= i) \
			{i = (ns) < j ? (ns) : j; \
			{WPUT(px, s, i)} \
			} \
		}	/* pad n elements from s[ns] */

#define WPUT(px, s, n)	\
	if (0 < (n)) \
		if (((px)->arg = (*(px)->pfn)((px)->arg, s, n)) != 0) \
			(px)->nchar += (n); \
		else \
			return (EOF);	/* put n elements from s */

		/* static data */
#define S	L' '
#define Z	L'0'
static const wchar_t spaces[] = {
	S,S,S,S,S,S,S,S, S,S,S,S,S,S,S,S,
	S,S,S,S,S,S,S,S, S,S,S,S,S,S,S,S};
static const wchar_t zeroes[] = {
	Z,Z,Z,Z,Z,Z,Z,Z, Z,Z,Z,Z,Z,Z,Z,Z,
	Z,Z,Z,Z,Z,Z,Z,Z, Z,Z,Z,Z,Z,Z,Z,S};

int _WPuttxt(_WPft *px, const wchar_t *ac)
	{	/* print generated wide text plus padding */
	int width = (((((px->width - px->n0) - px->nz0) - px->n1)
		- px->nz1) - px->n2) - px->nz2;

	if (!(px->flags & _FMI))
		{WPAD(px, &spaces[0], width, WMAX_PAD)}
	{WPUT(px, ac, px->n0)}
	{WPAD(px, &zeroes[0], px->nz0, WMAX_PAD)}
	{WPUT(px, px->s, px->n1)}
	{WPAD(px, &zeroes[0], px->nz1, WMAX_PAD)}
	{WPUT(px, px->s + px->n1, px->n2)}
	{WPAD(px, &zeroes[0], px->nz2, WMAX_PAD)}
	if (px->flags & _FMI)
		{WPAD(px, &spaces[0], width, WMAX_PAD)}
	return (0);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwputtxt.c $Rev: 153052 $");
