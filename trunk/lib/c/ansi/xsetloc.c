/* _Setloc function */
#include <ctype.h>
#include "xmtloc.h"
#include "xlocale.h"
_STD_BEGIN

_Linfo *_Setloc(size_t catidx, _Linfo *p)
	{	/* set category for locale */
	switch (catidx)
		{	/* set a category */
	case _LC_COLLATE:
		*_TLS_DATA_PTR(_Costate) = p->_Costate;
		*_TLS_DATA_PTR(_WCostate) = p->_WCostate;
		break;

	case _LC_CTYPE:
		*_TLS_DATA_PTR(_Ctype) = p->_Ctype;
		*_TLS_DATA_PTR(_Tolotab) = p->_Tolotab;
		*_TLS_DATA_PTR(_Touptab) = p->_Touptab;
		*_TLS_DATA_PTR(_Mbcurmax) = p->_Mbcurmax <= MB_LEN_MAX
			? p->_Mbcurmax : MB_LEN_MAX;
		*_TLS_DATA_PTR(_Mbstate) = p->_Mbstate;
		*_TLS_DATA_PTR(_Wcstate) = p->_Wcstate;
		*_TLS_DATA_PTR(_Wctrans) = p->_Wctrans;
		*_TLS_DATA_PTR(_Wctype) = p->_Wctype;
		break;

	case _LC_MONETARY:
		 {	/* set monetary category */
		struct lconv *plconv = _TLS_DATA_PTR(_Locale);

		plconv->currency_symbol = p->_Lc.currency_symbol;
		plconv->int_curr_symbol = p->_Lc.int_curr_symbol;
		plconv->mon_decimal_point = p->_Lc.mon_decimal_point;
		plconv->mon_grouping = p->_Lc.mon_grouping;
		plconv->mon_thousands_sep = p->_Lc.mon_thousands_sep;
		plconv->negative_sign = p->_Lc.negative_sign;
		plconv->positive_sign = p->_Lc.positive_sign;

		plconv->frac_digits = p->_Lc.frac_digits;
		plconv->n_cs_precedes = p->_Lc.n_cs_precedes;
		plconv->n_sep_by_space = p->_Lc.n_sep_by_space;
		plconv->n_sign_posn = p->_Lc.n_sign_posn;
		plconv->p_cs_precedes = p->_Lc.p_cs_precedes;
		plconv->p_sep_by_space = p->_Lc.p_sep_by_space;
		plconv->p_sign_posn = p->_Lc.p_sign_posn;

		plconv->int_frac_digits = p->_Lc.int_frac_digits;
/* vv added with C99 vv */
		plconv->int_n_cs_precedes = p->_Lc.int_n_cs_precedes;
		plconv->int_n_sep_by_space = p->_Lc.int_n_sep_by_space;
		plconv->int_n_sign_posn = p->_Lc.int_n_sign_posn;
		plconv->int_p_cs_precedes = p->_Lc.int_p_cs_precedes;
		plconv->int_p_sep_by_space = p->_Lc.int_p_sep_by_space;
		plconv->int_p_sign_posn = p->_Lc.int_p_sign_posn;
/* ^^ added with C99 ^^ */
		 }
		break;

	case _LC_NUMERIC:
		 {	/* set numeric category */
		struct lconv *plconv = _TLS_DATA_PTR(_Locale);

		plconv->decimal_point = p->_Lc.decimal_point[0] != '\0'
			? p->_Lc.decimal_point : (char *)".";
		plconv->grouping = p->_Lc.grouping;
		plconv->thousands_sep = p->_Lc.thousands_sep;
		plconv->_Frac_grouping = p->_Lc._Frac_grouping;
		plconv->_Frac_sep = p->_Lc._Frac_sep;
		plconv->_False = p->_Lc._False;
		plconv->_True = p->_Lc._True;
		 }
		break;

	case _LC_TIME:
		*_TLS_DATA_PTR(_Times) = p->_Times;
		break;
	case _LC_MESSAGES:
		 {	/* set messages category */
		struct lconv *plconv = _TLS_DATA_PTR(_Locale);

		plconv->_No = p->_Lc._No;
		plconv->_Yes = p->_Lc._Yes;
		 }
		break;
	default:
		break;
		}
	return (p);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xsetloc.c $Rev: 153052 $");
