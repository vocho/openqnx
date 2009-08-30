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

/* mktime function */
#include <inttypes.h>
#include <limits.h>
#include "xtime.h"
_STD_BEGIN

time_t (mktime)(struct tm *t)
	{	/* convert local time structure to scalar time */
#if _ADD_POSIX	/* __QNX__ */
	int64_t lsecs;
#else
	double dsecs;
#endif
	int mon, year, ymon;
	time_t secs;

#if _ADD_POSIX	/* __QNX__ */
	_Tzset();
#endif
	ymon = t->tm_mon / 12;
	mon = t->tm_mon - ymon * 12;
	if (mon < 0)
		mon += 12, --ymon;
	if (ymon < 0 && t->tm_year < INT_MIN - ymon
		|| 0 < ymon && INT_MAX - ymon < t->tm_year)
		return ((time_t)(-1));
	year = t->tm_year + ymon;
#if !_ADD_POSIX	/* __QNX__ */
	dsecs = 86400.0 * (_Daysto(year, mon) - 1)
		+ 31536000.0 * year + 86400.0 * t->tm_mday;
	dsecs += 3600.0 * t->tm_hour + 60.0 * t->tm_min
		+ (double)t->tm_sec;
	if (dsecs < 0.0)
		return ((time_t)(-1));
	secs = (time_t)(dsecs - (double)_TBIAS);

/*	allow wraparound times for now
 *	if (dsecs != (double)secs)
 *		return ((time_t)(-1));
 */
	_Ttotm(t, secs, t->tm_isdst);
	if (0 < t->tm_isdst)
		secs -= 3600;
	return (secs - _Tzoff());
#else 
	lsecs = 86400 * (_Daysto(year, mon) - 1) 
               + ((int64_t)31536000 * (int64_t)year) + 86400 * t->tm_mday;
    lsecs += (int64_t)3600 * t->tm_hour + 60 * t->tm_min + t->tm_sec;
	if ((lsecs < _TBIAS) || ((int64_t)(time_t)(-1) <= (lsecs - _TBIAS)))
		return ((time_t)(-1));
	secs = (time_t)(lsecs - _TBIAS); 
	/* At this point, secs is an accurate reflection of the t structure, except
	 * that the time zone hasn't been taken into account.  secs is accurate if
	 * the timezone is GMT only.  We can use _Ttotm to calculate the offset from
	 * GMT
	 */
	(void)_Ttotm(t, secs, t->tm_isdst, __MKTIME_CALL);
	secs -= t->tm_gmtoff;
	/* Finally, update the fields of t */
	(void)_Ttotm(t, secs, -1, __LOCALTIME_CALL);
	return(secs);
#endif
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("mktime.c $Rev: 153052 $");
