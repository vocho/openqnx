/*
 * $QNXLicenseC:
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




#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "xtime.h"

_STD_BEGIN

#if 1

#ifdef __QNX__
const char *__getsecs(const char *strp, long *secsp);

static int numlen(long num)
{
  long rem;
  int count=0;
  while (num) {
    rem = num%10;
    num = (num-rem)/10;
    count++;
  }
  return(count);
}

#endif

static const char *gettz(const char *s, char **p) {
	int			quoted;
	char		c;
	int			len;

	quoted = 0;
	if(*s == '<') {
		quoted = *s++;
	}
	len = 0;
	while((c = *s) && (quoted ? c != '>' : !isdigit(c) && c != ',' && c != '-' && c != '+')) {
		if(len < TZNAME_MAX) {
			*(*p)++ = c == ':' ? ' ' : c;
			len++;
		}
		s++;
	}
	*(*p)++ = ':';
	return quoted ? ++s : s;
}

#ifdef __QNX__
static const char *getoff(const char *s, char **p) {
  int  len;
  long secs;
  const char *temp;
  int i;

  if(*s != '-') {
    *(*p)++ = '-';
  }
  if(*s == '+' || *s == '-') {
    s++;
  }
  temp = __getsecs(s, &secs); // convert HH[[:MM]:SS]] to secs
  if (temp == NULL) // error reading format, assume 0
    secs=0;
  len = numlen(secs);
  for (i=len-1; i >= 0 ; i--) {
    *((*p)+i) = secs%10+'0';
    secs = secs/10;
  }
  *p += len;
  *(*p)++ = ':';
  return s;
}
#else
static const char *getoff(const char *s, char **p) {
	int			len;

	if(*s != '-') {
		*(*p)++ = '-';
	}
	if(*s == '+' || *s == '-') {
		s++;
	}
	while(*s == '0') {
		s++;
	}
	if(isdigit(*s)) {
		*(*p)++ = *s++;
	}
	if(isdigit(*s)) {
		*(*p)++ = *s++;
	}
	len = 2;
	if(*s == ':') {
		s++;
		while(*s == '0') {
			s++;
		}
		if(isdigit(*s)) {
			*(*p)++ = *s++;
			len--;
		}
		if(isdigit(*s)) {
			*(*p)++ = *s++;
			len--;
		}
	}
	while(len--) {
		*(*p)++ = '0';
	}
	*(*p)++ = ':';
	return s;
}

#endif

static char *reformat(const char *s) {
	char		c;
	static char	tzbuf[128];
	char		*p = tzbuf;
	const char	*off, *tmp;
#ifdef __QNX__
  long stdoff, dstoff;
  int len;
  char *stdoffbeg;
	const char *off1;
  int i;
#endif

	*p++ = ':';
	off = s = gettz(s, &p);
	while((c = *s) == '-' || c == '+' || c == ':' || isdigit(c)) {
		s++;
	}
	if(s == off || p == &tzbuf[1]) {
		*p = '\0';
		return tzbuf;
	}
	tmp = s;
#ifdef __QNX__
  off1 = s = gettz(s, &p);
  stdoffbeg = p; // save beginning of std offset
#else
	s = gettz(s, &p);
#endif
	(void)getoff(off, &p);
	if(tmp != s) {
		while((c = *s) == '-' || c == '+' || c == ':' || isdigit(c)) {
			s++;
		}
	}
	*p = '\0';
#ifdef __QNX__
  if (tmp == s)
    return(tzbuf); // no dst
  if (off1 == s) { // no dst offset specified, use default 1 hour from std
    stdoff = strtol(stdoffbeg, NULL, 10);
    dstoff = stdoff + 3600; // one hour ahead
    if (dstoff < 0) {
      *p++ = '-';
      dstoff = -dstoff;
    }
    len = numlen(dstoff);
    for (i=len-1; i >= 0 ; i--) {
      *(p+i) = dstoff%10+'0';
      dstoff = dstoff/10;
    }
    p += len;
    *p++ = ':';
  }
  else {
    (void)getoff(off1, &p);
  }
#endif
#ifdef __QNX__
  if(*s != ',') {  // dst specified, but no rules, setup default
    strcpy(p, "M3.2.0/2,M11.1.0/2");
    return tzbuf;
  }
  else { // *s == '.'
    s++; // skip ','
    strcpy(p, s);
  }
#else
	if(*s != ',') {
		if(tmp != s) {
			strcpy(p, "040102+0:103102-0");
		}
		return tzbuf;
	}
  if(tmp != s) 
    strcpy(p, "040102+0:103102-0");
#endif
	return tzbuf;
}
	

#else
static char *reformat(const char *s)
	{	/* reformat TZ from EST-05EDT */
	int i;
	static char tzbuf[] = ":EST:EDT:-0500:040102+0:103102-0";

	for (i = 1; i <= 3; ++i)
		if (isalpha(*s))
			tzbuf[i] = *s, tzbuf[i + 4] = *s++;
		else
			return (0);

	tzbuf[9] = *s == '-' || *s == '+' ? *s++ : '+';
	tzbuf[9] = tzbuf[9] == '+' ? '-' : '+';
	tzbuf[10] = tzbuf[11] = '0';
	if (isdigit(*s))
		tzbuf[11] = *s++;
	if (isdigit(*s))
		tzbuf[10] = tzbuf[11], tzbuf[11] = *s++;

	if (isalpha(*s))
		{ 
		tzbuf[14] = ':';
		for (i = 5; i <= 7; ++i)
			if (isalpha(*s))
				tzbuf[i] = *s++;
			else
				return (0);
		} else
			tzbuf[14] = '\0';
	return ((*s == '\0' || *s == ',') ? tzbuf : 0);
	}
#endif

static const char *defzone = ":";
static char *tzone;
static char *tzstr;

static int learnzone(void)
	{	/* get time zone information */
	const char *s;
	const char *tz;

	if ((tz = getenv("TZ")) == 0)
		{	/* Pick a default timezone */
#ifdef _CS_TIMEZONE
		size_t				len, len2;
		char				*tmp;

		for (;;) {
			if ((len = confstr(_CS_TIMEZONE, 0, 0)) > 0 && (tmp = alloca(len + 1)) != NULL) {
				if ((len2 = confstr(_CS_TIMEZONE, tmp, len + 1)) > len)
					continue;
				tz = tmp;
			}
			break;
		}
		if (!tz)
#endif
			tz = "UTC0";
		}

	if (tzstr && strcmp(tzstr, tz) == 0)
		return 1;

	if ((tzstr = (char *)realloc(tzstr, strlen(tz) + 1)) != 0)
		tz = strcpy(tzstr, tz);

	if (tzone && tzone != (char *)defzone)
		free(tzone);
	tzone = 0;

	if ((s = reformat(tz)))
		if ((tzone = (char *)malloc(strlen(s) + 1)) != 0)
			strcpy(tzone, s);
	
	if (tzone == 0)
		tzone = (char *)defzone;

	return 0;
	}

char	*tzname[2];
long	timezone;
int		daylight;

#ifdef __QNX__
char *(__Tzset)(void) {
#else
char *(_Tzset)(void) {
#endif
	int pn;
	const char *p;
#ifdef __QNX__
	int i;
	char *oldtz, *newtz;
#else
	char **tz;
#endif

	if(learnzone())
		return tzone;

	_Times._Tzone = "";
	_Times._Isdst = "";
	timezone = -_Tzoff();
	(void)_Getrules(NULL);

#ifdef __QNX__
	for (i = 0; i < 2; ++i) {
		oldtz = tzname[i];
		p = _Gettime(_Times._Tzone, i, &pn);
		if (pn <= 0)
			newtz = NULL;
		else if ((newtz = malloc(pn + 1)) != NULL)
			memcpy(newtz, p, pn), newtz[pn] = '\0';
		tzname[i] = (newtz != NULL) ? newtz : "";
		if (oldtz != NULL && *oldtz != '\0')
			free(oldtz);
	}
#else
	p = _Gettime(_Times._Tzone, 0, &pn);
	tz = &tzname[0];
	if(*tz && (*tz)[0])
		free(*tz);
	if(pn > 0 && (*tz = malloc(pn + 1)))
		memcpy(*tz, p, pn), (*tz)[pn] = '\0';
	else
		*tz = "";

	p = _Gettime(_Times._Tzone, 1, &pn);
	tz = &tzname[1];
	if(*tz && (*tz)[0])
		free(*tz);
	if(pn > 0 && (*tz = malloc(pn + 1)))
		memcpy(*tz, p, pn), (*tz)[pn] = '\0';
	else
		*tz = "";
#endif

	daylight = _Times._Isdst[0] != '\0' && pn > 0;

	return tzone;
}

#ifdef __QNX__
#include <pthread.h>
#include <stdio.h>
static pthread_mutex_t tzset_mutex = PTHREAD_RMUTEX_INITIALIZER;

char *(_Tzset)(void) {
char *tzsetp;

	if (_Multi_threaded) pthread_mutex_lock(&tzset_mutex);
	tzsetp = __Tzset();
	if (_Multi_threaded) pthread_mutex_unlock(&tzset_mutex);
	return(tzsetp);
}
#endif

_STD_END

__SRCVERSION("xtzset.c $Rev: 171665 $");
