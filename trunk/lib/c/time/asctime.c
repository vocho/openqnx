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





/* asctime function */
#include "xtime.h"
#include "xtls.h"
_STD_BEGIN

		/* static data */
static const char alt_digits[] = "";
static const char am_pm[] = ":AM:PM";
static const char days[] =
	":Sun:Sunday:Mon:Monday:Tue:Tuesday:Wed:Wednesday"
	":Thu:Thursday:Fri:Friday:Sat:Saturday";
static const char era[] = "";
static const char fmts[] =
	"|%a %b %e %T %Y|%m/%d/%y|%H:%M:%S|%I:%M:%S %p";
static const char isdst[] = "";
static const char mons[] =
	":Jan:January:Feb:February:Mar:March"
	":Apr:April:May:May:Jun:June"
	":Jul:July:Aug:August:Sep:September"
	":Oct:October:Nov:November:Dec:December";
static const char tzone[] = "";	/* adapt by default */

static _Tinfo _CTinfo = {	/* time info for C locale */
	am_pm,
	days,	/* days, abday, day */
		days, days,
	mons,	/* months, abmon, mon */
		mons, mons,
	fmts,	/* formats, d_t_fmt, d_fmt, t_fmt, t_fmt_am_pm */
		fmts, fmts, fmts, fmts,
	fmts,	/* as above, for era */
		fmts, fmts, fmts, fmts,
	era,	/* era */
	alt_digits,	/* alt_digits */
	isdst,	/* isdst */
	tzone	/* tzone */
	};

_TLS_DEFINE_INIT(_IMPLICIT_EXTERN, _Tinfo, _Times) =
	{	/* time info for current locale */
	am_pm,
	days,	/* days, abday, day */
		days, days,
	mons,	/* months, abmon, mon */
		mons, mons,
	fmts,	/* formats, d_t_fmt, d_fmt, t_fmt, t_fmt_am_pm */
		fmts, fmts, fmts, fmts,
	fmts,	/* as above, for era */
		fmts, fmts, fmts, fmts,
	era,	/* era */
	alt_digits,	/* alt_digits */
	isdst,	/* isdst */
	tzone	/* tzone */
	};
_TLS_DEFINE_NO_INIT(_IMPLICIT_EXTERN, _Tinfo, _Times);

_Tinfo *(_Getptimes)(void)
	{	/* get pointer to _Times, locked */
	return (_TLS_DATA_PTR(_Times));
	}

#define TBUF_SIZE	sizeof("Day Mon dd hh:mm:ss yyyy\n")

_TLS_ARR_DEF(static, char, tbuf, TBUF_SIZE);

char *(asctime)(const struct tm *t)
	{	/* format time as "Day Mon dd hh:mm:ss yyyy\n" */
	char *ptr = _TLS_ARR(tbuf);

	(void)_CStrftime(ptr, TBUF_SIZE, "%c\n", t, &_CTinfo);
	return (ptr);
	}

#if _ADD_POSIX	/* __QNX__ */
char *(asctime_r)(const struct tm *t, char *tbufp)
	{	/* format time as "Day Mon dd hh:mm:ss yyyy\n" */
	(void)_CStrftime(tbufp, TBUF_SIZE, "%c\n", t, &_CTinfo);
	return (tbufp);
	}
#endif
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("asctime.c $Rev: 153052 $");
