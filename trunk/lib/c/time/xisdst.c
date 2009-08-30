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

/*
** This source code contains portions of a file 
** that is in the public domain, so clarified as 
** of 1996-06-05 by Arthur David Olson 
** (arthur_david_olson@nih.gov).
*/

/*
** Leap second handling from Bradley White (bww@k.gp.cs.cmu.edu).
** POSIX-style TZ environment variable handling from Guy Harris
** (guy@auspex.com).
*/

/* _Isdst function */
#include <stdlib.h>
#include "xmtloc.h"
#include "xtls.h"
#include "xtime.h"
_STD_BEGIN

#ifdef __QNX__
static const int  mon_lengths[2][MONSPERYEAR] = {
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
#else
typedef const char *dst_t;
typedef Dstrule *rules_t;

static void free_tls(void *ptr)
	{	/* free any storage pointed at by thread-local rules */
	rules_t *prules = (rules_t *)ptr;

	free(*prules);
	}

_TLS_DATA_DEF(static, dst_t, olddst, 0);
_TLS_DATA_DEF_DT(static, rules_t, rules, 0, free_tls);
#endif

#ifdef __QNX__
int _Isdst(const struct tm *t, long tzoff, long tzoffdst)
#else
int _Isdst(const struct tm *t)
#endif
	{	/* test whether Daylight Savings Time in effect */
#ifdef __QNX__
	Dstrule *rules;
	int  dst_state;
	
	if ((rules = _Getrules(&dst_state)) == NULL)
		return dst_state;
#else
	Dstrule *pr;
	dst_t *polddst = _TLS_DATA_PTR(olddst);
	rules_t *prules = _TLS_DATA_PTR(rules);
	_Tinfo *ptimes = _TLS_DATA_PTR(_Times);

	if (*polddst != ptimes->_Isdst)
		{	/* find current dst_rules */
		if (ptimes->_Isdst[0] == '\0')
			{	/* look beyond time_zone info */
			int n;

			if (ptimes->_Tzone[0] == '\0')
				ptimes->_Tzone = _Getzone();
			ptimes->_Isdst = _Gettime(ptimes->_Tzone, 3, &n);
			if (ptimes->_Isdst[0] != '\0')
				--ptimes->_Isdst;	/* point to delimiter */
			}
		if ((pr = _Getdst(ptimes->_Isdst)) == 0)
			return (-1);
		free(*prules);
		*prules = pr;
		*polddst = ptimes->_Isdst;
		}
#endif
	 {	/* check time against rules */
#ifdef __QNX__
  	int ans   = 0;
    int south = 0;  // southern hemisphere (where start > end)
    int START = 0, END = 1;
		long seconds[2]; // in secs, from beginning of year
		int leapyear;
		int i,j;	
		int d, m1, yy0, yy1, yy2, dow;
		long currsecs; // seconds from beginning of year for current time

		seconds[0] = 0L;
		seconds[1] = 0L;
		leapyear = ISLEAP(t->tm_year+1900);
   	for (i=0; i < 2; i++) {
			switch ((rules[i]).rtype) {
				case JTYPE: // Julian day type 
          seconds[i] = ((rules[i]).day - 1) * SECSPERDAY;
          if (leapyear && (rules[i]).day >= 60)
            seconds[i] += SECSPERDAY;
					break;
				case ZTYPE: // Zero Julian day type
          seconds[i] = (rules[i]).day * SECSPERDAY;
          break;
				case MTYPE: // Month type
          seconds[i] = 0;
          for (j = 0; j < (rules[i]).mon - 1; ++j)
            seconds[i] += mon_lengths[leapyear][j] * SECSPERDAY;
          /*
          ** Use Zeller's Congruence to get day-of-week of first day of
          ** month.
          */
          m1 = ((rules[i]).mon + 9) % 12 + 1;
          yy0 = ((rules[i]).mon <= 2) ? (t->tm_year - 1) : t->tm_year;
					yy0 += 1900; // normalise year 
          yy1 = yy0 / 100;
          yy2 = yy0 % 100;
          dow = ((26 * m1 - 2) / 10 +
            1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
          if (dow < 0)
            dow += DAYSPERWEEK;
          /*
          ** "dow" is the day-of-week of the first day of the month.  Get
          ** the day-of-month (zero-origin) of the first "dow" day of the
          ** month.
          */
          d = (rules[i]).day - dow;
          if (d < 0)
            d += DAYSPERWEEK;
          for (j = 1; j < (rules[i]).week; ++j) {
            if (d + DAYSPERWEEK >= mon_lengths[leapyear][(rules[i]).mon - 1])
                break;
            d += DAYSPERWEEK;
          }
          /*
          ** "d" is the day-of-month (zero-origin) of the day we want.
          */
          seconds[i] += d * SECSPERDAY;
					break;
				default:
					break;
			}
			seconds[i] += (rules[i]).secs;
		}
		// check to see if start > end 
		if (seconds[START] > seconds[END])
			south=1; // in southern hemisphere
		currsecs = 0;
		currsecs += t->tm_yday * SECSPERDAY;
		currsecs += t->tm_hour * SECSPERHOUR;
		currsecs += t->tm_min  * SECSPERMIN;
		currsecs += t->tm_sec;
    if (!south) {  // if curr > start  && curr < end -> dst=1
			if (currsecs >= seconds[START]) {
				// possibly in dst
				// convert currsecs to be dst corrected
				currsecs -= tzoff;
				currsecs += tzoffdst;
				if (currsecs < seconds[END]) {
					ans=1;
				}
			}
		}
    // in southern hemisphere
		else {         // if curr > start  || curr < end -> dst=1
      // since start > end
			if ((currsecs >= seconds[START]) || (currsecs < seconds[END])) {
				if (currsecs >= seconds[START])
					ans=1;
				else {
					// possibly in dst
					// convert currsecs to be dst corrected
					currsecs -= tzoff;
					currsecs += tzoffdst;
					if (currsecs < seconds[END]) {
						ans=1;
					}
				}
			}
		}
#else
	int ans = 0;
	const int d0 = _Daysto(t->tm_year, 0);
	const int hour = t->tm_hour + 24 * t->tm_yday;
	const int wd0 = (365L * t->tm_year + d0 + WDAY) % 7 + 14;

	for (pr = *prules; pr->wday != (unsigned char)-1; ++pr)
		if (pr->year <= t->tm_year)
			{	/* found early enough year */
			int rday = _Daysto(t->tm_year, pr->mon) - d0
				 + pr->day;

			if (0 < pr->wday)
				{	/* shift to specific weekday */
				int wd = (rday + wd0 - pr->wday) % 7;

				rday += wd == 0 ? 0 : 7 - wd;
				if (pr->wday <= 7)
					rday -= 7;	/* strictly before */
				}
			if (hour < rday * 24 + pr->hour)
				return (ans);
			ans = pr->year == (pr + 1)->year ? !ans : 0;
			}
#endif
	return (ans);
	 }
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xisdst.c $Rev: 153052 $");
