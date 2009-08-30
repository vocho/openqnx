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

/* xtime.h internal header */
#ifndef _XTIME
#define _XTIME
#include <xtinfo.h>
#ifdef __QNX__
#include "yvals.h"
#endif
_C_STD_BEGIN

		/* macros */
#define WDAY	1	/* to get day of week right */

#ifdef __QNX__
#define SECSPERMIN  60
#define MINSPERHOUR 60
#define HOURSPERDAY 24
#define SECSPERHOUR (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY  ((long) SECSPERHOUR * HOURSPERDAY)
#define DAYSPERYEAR 365
#define DAYSPERWEEK 7
#define MONSPERYEAR 12

#define ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

/* types of rules (for dst) */
enum ruletype {
  JTYPE=1, /* julian day type */
  ZTYPE,   /* zero based julian day type  */
  MTYPE    /* d'th day type */
};

/* type definitions */
typedef struct
{  /* rule for daylight savings time */
    int rtype; /* type of rule */
    int mon;   /* month number */
    int week;  /* week number */
    int day;   /* day number */
    long secs; /* time in secs since beginning of day */
} Dstrule;

#else
		/* type definitions */
typedef struct
	{	/* rule for daylight savings time */
	unsigned char wday, hour, day, mon, year;
	} Dstrule;
#endif

		/* internal declarations */
_C_LIB_DECL
int _Daysto(int, int);
const char * _Gentime(const struct tm *, const _Tinfo *,
	char, char, int *, char *);
Dstrule * _Getdst(const char *);
const char * _Gettime(const char *, int, int *);
#ifdef __QNX__
int _Isdst(const struct tm *, long tzoff, long tzoffdst);
#else
int _Isdst(const struct tm *);
#endif
const char *_Getzone(void);
#ifdef __QNX__
char *(_Tzset)(void);
Dstrule *_Getrules(int *);
enum {
	__GMTIME_CALL=0,
	__LOCALTIME_CALL,
	__MKTIME_CALL
};
struct tm *_Ttotm(struct tm *, time_t, int, int);
#else
struct tm *_Ttotm(struct tm *, time_t, int);
#endif
#ifndef __QNX__
time_t _Tzoff(void);
#else
long _Tzoff(void);
long _Tzoff_dst(void);
#endif
_END_C_LIB_DECL
_C_STD_END
#endif /* _XTIME */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xtime.h $Rev: 153052 $"); */
