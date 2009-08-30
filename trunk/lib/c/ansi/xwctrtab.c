/* xwctrtab conversion table -- ASCII based version */
 #include "xmtloc.h"
_STD_BEGIN

	/* static data */
static const wchar_t tr_range_tab[] = {
	L'A', L'Z', L'a' - L'A',				/* 0: towlower */
	L'a', L'z', (wchar_t)(L'A' - L'a')};	/* 3: towupper */

static const _Wctab wctrans_tab[] = {
	{(const char *)tr_range_tab, 0},	/* table pointer, allocated size */
	{"tolower", 0},
	{"toupper", 3},
	{(const char *)0, 6}};	/* null pointer, first unused offset */

_TLS_DATA_DEF(_IMPLICIT_EXTERN, _PWctab, _Wctrans, &wctrans_tab[0]);

const _Wctab *_Getpwctrtab()
	{	/* get pointer to _Wctrans table */
	return (*_TLS_DATA_PTR(_Wctrans));
	}_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwctrtab.c $Rev: 153052 $");
