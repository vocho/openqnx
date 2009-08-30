/*
 * $QNXtpLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */



/*




Also copyright P.J. Plauger - see bottom of file for details.
*/

/* _Loctab data object */
#include <stddef.h>
#include "xlocale.h"
_STD_BEGIN

		/* macros */
#define OFF(member)		offsetof(_Linfo, member)

		/* static data */
/* extern const */ _Locitem _Loctab[] = {	/* locale file info */
	{"LOCALE", OFF(_Name), L_NAME},
	{"NOTE", 0, L_NOTE},
	{"SET", 0, L_SETVAL},

		/* controlled by LC_COLLATE */
	{"collate", OFF(_Costate._Tab), L_STATE},
	{"wcollate", OFF(_WCostate._Tab), L_STATE},

		/* controlled by LC_CTYPE */
	{"ctype", OFF(_Ctype), L_TABLE},
	{"tolower", OFF(_Tolotab), L_TABLE},
	{"toupper", OFF(_Touptab), L_TABLE},
	{"mb_cur_max", OFF(_Mbcurmax), L_VALUE},
	{"mbtowc", OFF(_Mbstate._Tab), L_STATE},
	{"wctomb", OFF(_Wcstate._Tab), L_STATE},
	{"wctrans", OFF(_Wctrans), L_WCTYPE},
	{"wctype", OFF(_Wctype), L_WCTYPE},

		/* controlled by LC_MONETARY */
	{"currency_symbol", OFF(_Lc.currency_symbol), L_STRING},
	{"int_curr_symbol", OFF(_Lc.int_curr_symbol), L_STRING},
	{"mon_decimal_point", OFF(_Lc.mon_decimal_point), L_STRING},
	{"mon_grouping", OFF(_Lc.mon_grouping), L_GSTRING},
	{"mon_thousands_sep", OFF(_Lc.mon_thousands_sep), L_STRING},
	{"negative_sign", OFF(_Lc.negative_sign), L_STRING},
	{"positive_sign", OFF(_Lc.positive_sign), L_STRING},

	{"frac_digits", OFF(_Lc.frac_digits), L_VALUE},
	{"n_cs_precedes", OFF(_Lc.n_cs_precedes), L_VALUE},
	{"n_sep_by_space", OFF(_Lc.n_sep_by_space), L_VALUE},
	{"n_sign_posn", OFF(_Lc.n_sign_posn), L_VALUE},
	{"p_cs_precedes", OFF(_Lc.p_cs_precedes), L_VALUE},
	{"p_sep_by_space", OFF(_Lc.p_sep_by_space), L_VALUE},
	{"p_sign_posn", OFF(_Lc.p_sign_posn), L_VALUE},

	{"int_frac_digits", OFF(_Lc.int_frac_digits), L_VALUE},
/* vv added with C99 vv */
	{"int_n_cs_precedes", OFF(_Lc.int_n_cs_precedes), L_VALUE},
	{"int_n_sep_by_space", OFF(_Lc.int_n_sep_by_space), L_VALUE},
	{"int_n_sign_posn", OFF(_Lc.int_n_sign_posn), L_VALUE},
	{"int_p_cs_precedes", OFF(_Lc.int_p_cs_precedes), L_VALUE},
	{"int_p_sep_by_space", OFF(_Lc.int_p_sep_by_space), L_VALUE},
	{"int_p_sign_posn", OFF(_Lc.int_p_sign_posn), L_VALUE},
/* ^^ added with C99 ^^ */

		/* controlled by LC_NUMERIC */
	{"decimal_point", OFF(_Lc.decimal_point), L_STRING},
	{"grouping", OFF(_Lc.grouping), L_GSTRING},
	{"thousands_sep", OFF(_Lc.thousands_sep), L_STRING},
	{"frac_grouping", OFF(_Lc._Frac_grouping), L_GSTRING},
	{"frac_sep", OFF(_Lc._Frac_sep), L_STRING},
	{"false", OFF(_Lc._False), L_STRING},
	{"true", OFF(_Lc._True), L_STRING},

		/* controlled by LC_TIME */
	{"am_pm", OFF(_Times._Am_pm), L_STRING},
	{"days", OFF(_Times._Days), L_STRING},
		{"abday", OFF(_Times._Abday), L_STRING},
		{"day", OFF(_Times._Day), L_STRING},
	{"months", OFF(_Times._Months), L_STRING},
		{"abmon", OFF(_Times._Abmon), L_STRING},
		{"mon", OFF(_Times._Mon), L_STRING},
	{"time_formats", OFF(_Times._Formats), L_STRING},
		{"d_t_fmt", OFF(_Times._D_t_fmt), L_STRING},
		{"d_fmt", OFF(_Times._D_fmt), L_STRING},
		{"t_fmt", OFF(_Times._T_fmt), L_STRING},
		{"t_fmt_ampm", OFF(_Times._T_fmt_ampm), L_STRING},
	{"era_time_formats", OFF(_Times._Era_Formats), L_STRING},
		{"era_d_t_fmt", OFF(_Times._Era_D_t_fmt), L_STRING},
		{"era_d_fmt", OFF(_Times._Era_D_fmt), L_STRING},
		{"era_t_fmt", OFF(_Times._Era_T_fmt), L_STRING},
		{"era_t_fmt_ampm", OFF(_Times._Era_T_fmt_ampm), L_STRING},
	{"era", OFF(_Times._Era), L_STRING},
	{"alt_digits", OFF(_Times._Alt_digits), L_STRING},
	{"dst_rules", OFF(_Times._Isdst), L_STRING},
	{"time_zone", OFF(_Times._Tzone), L_STRING},

		/* controlled by LC_MESSAGES */
	{"no", OFF(_Lc._No), L_STRING},
	{"yes", OFF(_Lc._Yes), L_STRING},
#if _ADD_POSIX	/* __QNX__ */
	{"nostr", OFF(_Lc._Nostr), L_STRING},
	{"yesstr", OFF(_Lc._Yesstr), L_STRING},
#endif /* _ADD_POSIX */
	{0}};
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xloctab.c $Rev: 153052 $");
