/* xwctytab conversion table -- ASCII based version */
#include "xmtloc.h"
_STD_BEGIN

	/* static data */
static const wchar_t ty_range_tab[] = {
	L'A', L'Z', 1,	/* 0: iswalnum */
	L'a', L'z', 1,
	L'0', L'9', 1,
	L'A', L'Z', 1,	/* 9: iswalpha */
	L'a', L'z', 1,
	0x01, 0x1f, 1,			/* 15: iswcntrl */
	L'0', L'9', 1,	/* 18: iswdigit */
	0x21, 0x7e, 1,			/* 21: iswgraph */
	L'a', L'z', 1, 	/* 24: iswlower */
	0x20, 0x7e, 1,			/* 27: iswprint */
	0x21, 0x2f, 1,			/* 30: iswpunct */
	0x3a, 0x40, 1,
	0x5b, 0x60, 1,
	0x7b, 0x7e, 1,
	L' ', L' ', 1,	/* 42: iswspace */
	0x09, 0x0d, 1,
	L'A', L'Z', 1,	/* 48: iswupper */
	L'0', L'9', 1,	/* 51: iswxdigit */
	L'a', L'f', 1,
	L'A', L'F', 1,
	L' ', L' ', 1,	/* 60: iswblank */
	L'\t', L'\t', 1,
	0x09, 0x0d, 1};

static const _Wctab wctype_tab[] = {
	{(const char *)ty_range_tab, 0},	/* table pointer, allocated size */
	{"alnum", 0},
	{"alpha", 9},
	{"cntrl", 15},
	{"digit", 18},
	{"graph", 21},
	{"lower", 24},
	{"print", 27},
	{"punct", 30},
	{"space", 42},
	{"upper", 48},
	{"xdigit", 51},
	{"blank", 60},
	{(const char *)0, 69}};	/* null pointer, first unused offset */

_TLS_DATA_DEF(_IMPLICIT_EXTERN, _PWctab, _Wctype, &wctype_tab[0]);

const _Wctab *_Getpwctytab()
	{	/* get pointer to _Wctype table */
	return (*_TLS_DATA_PTR(_Wctype));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwctytab.c $Rev: 153052 $");
