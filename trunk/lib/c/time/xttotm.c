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

/* _Ttotm and _Daysto functions */
#include "xtime.h"
#include "xtls.h"
#include <inttypes.h>

_STD_BEGIN

		/* macros */
#ifndef __QNX__
#define MONTAB(year)	\
	((year) & 03 || (year) == 0 ? mos : lmos)
#else
#define MONTAB(year) ( !ISLEAP(1900+year) ? mos : lmos)
#endif

		/* static data */
static const short lmos[] = {0, 31, 60, 91, 121, 152,
	 182, 213, 244, 274, 305, 335};
static const short mos[] = {0, 31, 59, 90, 120, 151,
	 181, 212, 243, 273, 304, 334};

#ifndef __QNX__
_TLS_DATA_DEF(static, struct tm, ts, {0});
#endif

int _Daysto(int year, int mon)
	{	/* compute extra days to start of month */
	int days;

#ifndef __QNX__
	if (0 < year)	/* correct for leap year: 1801-2099 */
		days = (year - 1) / 4;
#else
	if (0 < year)	
		days = ((year - 1) / 4 - ((year-1) / 100)) + (((year-1) + 300) / 400);
#endif
	else if (year <= -4)
		days = 1 + (4 - year) / 4;
	else
		days = 0;
	return (days + MONTAB(year)[mon]);
	}

#ifdef __QNX__
static struct tm *adjust_tm(uint64_t secs, struct tm *t)
{
	long days;
	int year;
	days = secs / 86400;
	t->tm_wday = (days + WDAY) % 7;
  {	/* determine year */
	  long i;

	  for (year = days / 365; days < (i = _Daysto(year, 0) + 365L * year); )
			  --year;	// correct guess and recheck 
	  days -= i; 
	  t->tm_year = year;
	  t->tm_yday = days;
  }
	{	/* determine month */
	  int mon;
	  const short *pm = MONTAB(year);
	  for (mon = 12; days < pm[--mon]; )
		  ;
	  t->tm_mon = mon;
	  t->tm_mday = (days - pm[mon]) + 1;
	}
	secs %= 86400;
	t->tm_hour = secs / 3600;
	secs %= 3600;
	t->tm_min = secs / 60;
	t->tm_sec = secs % 60;
	return(t);
}
#endif /* __QNX__ */

#ifdef __QNX__
struct tm *_Ttotm(struct tm *t, time_t secsarg, int isdst, int ct)
#else
struct tm *_Ttotm(struct tm *t, time_t secsarg, int isdst)
#endif
	{	/* convert scalar time to time structure */
#ifdef __QNX__
	long tzoffdst;
	long tzoff;
	static struct tm ts;
	uint64_t lsecsarg;
	uint64_t secs;

	lsecsarg = _TBIAS + (uint64_t)secsarg;
	tzoffdst = _Tzoff_dst();
	if (t == 0)
		t = &ts;
	t->tm_isdst = isdst;
	switch (ct) {
		case __GMTIME_CALL:
			t = adjust_tm(lsecsarg, t);
			t->tm_zone = tzname[(0 < t->tm_isdst ? 1 : 0)];
			t->tm_gmtoff = 0;
			break;
		case __LOCALTIME_CALL:
			tzoff = _Tzoff();
			secs = lsecsarg + tzoff;
			t = adjust_tm(secs, t);
			t->tm_isdst = _Isdst(t, tzoff, tzoffdst); 
			if (t->tm_isdst > 0) { // 
				secs = lsecsarg + tzoffdst;
				t = adjust_tm(secs, t);
				t->tm_gmtoff = tzoffdst;
			}
			else 
				t->tm_gmtoff = tzoff;
			t->tm_zone = tzname[(0 < t->tm_isdst ? 1 : 0)];
			break;
		case __MKTIME_CALL:
			tzoff = _Tzoff();
			t = adjust_tm(lsecsarg, t);
			if (isdst == 0) { // isdst == 0
				t->tm_isdst = isdst;
			}
			else {
				int  dst_state;
				if (_Getrules(&dst_state) == NULL)
					t->tm_isdst = 0;
				else	if (isdst <= 0)
					t->tm_isdst = _Isdst(t, 0, 0);
			}

			/* set up proper zone */
			t->tm_zone = tzname[(0 < t->tm_isdst ? 1 : 0)];
			if (t->tm_isdst > 0) // dst is on
				t->tm_gmtoff = tzoffdst;
			else 
				t->tm_gmtoff = tzoff;
			break;
		default:
			break;
	}	
 	return(t);
#else
	int year;
	long days;
	unsigned long secs;

	secsarg += _TBIAS;	/* changed to (wraparound) time since 1 Jan 1900 */
	if (t == 0)
		t = _TLS_DATA_PTR(ts);
	t->tm_isdst = isdst;
	for (secs = (unsigned long)secsarg; ;
		secs = (unsigned long)secsarg + 3600)
		{	/* loop to correct for DST */
		days = secs / 86400;
		t->tm_wday = (days + WDAY) % 7;
		 {	/* determine year */
		long i;

		for (year = days / 365;
			days < (i = _Daysto(year, 0) + 365L * year); )
			--year;	/* correct guess and recheck */
		days -= i;
		t->tm_year = year;
		t->tm_yday = days;
		 }
		 {	/* determine month */
		int mon;
		const short *pm = MONTAB(year);

		for (mon = 12; days < pm[--mon]; )
			;
		t->tm_mon = mon;
		t->tm_mday = days - pm[mon] + 1;
		 }
		secs = secs % 86400;
		t->tm_hour = secs / 3600;
		secs %= 3600;
		t->tm_min = secs / 60;
		t->tm_sec = secs % 60;
		if (0 <= t->tm_isdst || (t->tm_isdst = _Isdst(t)) <= 0)
			return (t);	/* loop only if <0 => 1 */
		}
#endif
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xttotm.c $Rev: 153052 $");
