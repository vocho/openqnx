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

/* _Getdst function */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "xtime.h"
_STD_BEGIN

#ifdef __QNX__
/*
** Given a pointer into a time zone string, extract a number from that string.
** Check that the number is within a specified range; if it is not, return
** NULL.
** Otherwise, return a pointer to the first character not part of the number.
*/

static const char *getnum(const char *strp, int *nump, 
                           const int min, const int max)
{
  register char c;
  register int  num;

  if (strp == NULL || !isdigit(c = *strp))
    return NULL;
  num = 0;
  do {
    num = num * 10 + (c - '0');
    if (num > max)
      return NULL;  /* illegal value */
    c = *++strp;
  } while (isdigit(c));
  if (num < min)
    return NULL;    /* illegal value */
  *nump = num;
  return strp;
}

/*
** Given a pointer into a time zone string, extract a number of seconds,
** in hh[:mm[:ss]] form, from the string.  If any error occurs, return NULL.
** Otherwise, return a pointer to the first character not part of the number
** of seconds.
*/

const char * __getsecs(const char *strp, long *secsp)
{
  int num;

  strp = getnum(strp, &num, 0, HOURSPERDAY);
  if (strp == NULL)
    return NULL;
  *secsp = num * (long) SECSPERHOUR;
  if (*strp == ':') {
    ++strp;
    strp = getnum(strp, &num, 0, MINSPERHOUR - 1);
    if (strp == NULL)
      return NULL;
    *secsp += num * SECSPERMIN;
    if (*strp == ':') {
      ++strp;
      /* `SECSPERMIN' allows for leap seconds.  */
      strp = getnum(strp, &num, 0, SECSPERMIN);
      if (strp == NULL)
        return NULL;
      *secsp += num;
    }
  }
  return strp;
}

Dstrule *_Getdst(const char *s)
{	/* parse DST rules */
	const char delim = *s++;
	Dstrule *pr, *rules;
	int i;

	if (delim == '\0')
		return (0);
	/* buy space for 2 rules */
  // one for the start dst, and the other for end dst
	if ((rules = (Dstrule *)malloc(sizeof (Dstrule) * 2)) == 0)
		return (0);
	pr = rules;
	/* parse rules */
  for (i=0; i < 2; i++, s++) {
    if (*s == 'J') { // Julian Day Type rule
      (rules[i]).rtype = JTYPE;
      ++s;
      s = getnum(s, &((rules[i]).day), 1, DAYSPERYEAR);			
    } else if (*s == 'M') { // Month type rule
      (rules[i]).rtype = MTYPE;
      ++s;
      s = getnum(s, &((rules[i]).mon), 1, MONSPERYEAR);
      if (s == NULL) {
				free(rules);
        return NULL;
			}
      if (*s++ != '.') {
				free(rules);
        return NULL;
			}
      s = getnum(s, &((rules[i]).week), 1, 5);
      if (s == NULL) {
				free(rules);
        return NULL;
			}
      if (*s++ != '.') {
				free(rules);
        return NULL;
			}
      s = getnum(s, &((rules[i]).day), 0, DAYSPERWEEK - 1);  
	  } else if (isdigit(*s)) { // Zero based Julian day type rule
      (rules[i]).rtype = ZTYPE;
      s = getnum(s, &((rules[i]).day), 0, DAYSPERYEAR);
    } else { // invalid type
      free(rules);
		  return(NULL);
	  }
    if (s == NULL) {
      free(rules);
      return NULL;
		}
    if (*s == '/') { // Time specified.
      ++s;
      s = __getsecs(s, &((rules[i]).secs));
      if (s == NULL) {
        free(rules);
        return NULL;
      }
    } else {
      (rules[i]).secs = 2 * SECSPERHOUR;  // default = 2:00:00
    }
    if (i == 0) { // first rule, must be terminated by ','
			if (*s != ',') {
				free(rules);
				return(NULL);
			}
		}
  }
	return(rules); // return the rules
}

#else
static int getint(const char *s, int n)
	{	/* accumulate digits */
	int value;

	for (value = 0; 0 <= --n && isdigit((unsigned char)*s); ++s)
		value = value * 10 + (unsigned char)*s - '0';
	return (0 <= n ? -1 : value);
	}

Dstrule *_Getdst(const char *s)
	{	/* parse DST rules */
	const char delim = *s++;
	Dstrule *pr, *rules;

	if (delim == '\0')
		return (0);
	 {	/* buy space for rules */
	const char *s1, *s2;
	int i;

	for (s1 = s, i = 2; (s2 = strchr(s1, delim)) != 0; ++i)
			s1 = s2 + 1;
	if ((rules = (Dstrule *)malloc(sizeof (Dstrule) * i))
		== 0)
		return (0);
	 }
	 {	/* parse rules */
	int year = 0;

	for (pr = rules; ; ++pr, ++s)
		{	/* parse next rule */
		if (*s == '(')
			{	/* got a year qualifier */
			year = getint(s + 1, 4) - 1900;
			if (year < 0 || s[5] != ')')
				break;	/* invalid year */
			s += 6;
			}
		pr->year = year;
		pr->mon = getint(s, 2) - 1, s += 2;
		pr->day = getint(s, 2) - 1, s += 2;
		if (isdigit((unsigned char)*s))
			pr->hour = getint(s, 2), s += 2;
		else
			pr->hour = 0;
		if (12 <= pr->mon || 99 < pr->day || 99 < pr->hour)
			break;	/* invalid month, day, or hour */
		if (*s != '+' && *s != '-')
			pr->wday = 0;
		else if (s[1] < '0' || '6' < s[1])
			break;	/* invalid week day */
		else
			{	/* compute week day field */
			pr->wday = s[1] == '0' ? 7 : s[1] - '0';
			if (*s == '+')	/* '-': strictly before */
				pr->wday += 7;	/* '+': on or after */
			s += 2;
			}
		if (*s == '\0')
			{	/* done, terminate list */
			(pr + 1)->wday = (unsigned char)-1;
			(pr + 1)->year = year;
			return (rules);
			}
		else if (*s != delim)
			break;
		}
	free(rules);
	return (0);
	 }
	}
#endif
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xgetdst.c $Rev: 153052 $");
