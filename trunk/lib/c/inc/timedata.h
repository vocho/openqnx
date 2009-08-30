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




#ifndef TIMEDATA_H_INCLUDED
#define TIMEDATA_H_INCLUDED

#include <time.h>

extern int	   __dst_adjust;
extern struct tm __start_dst;	/* start of daylight savings */
extern struct tm __end_dst;	/* end of daylight savings */

#define SECONDS_FROM_1900_TO_1970	2208988800UL
#define SECONDS_PER_DAY 		(24*60*60UL)
#define DAYS_FROM_1900_TO_1970		((long)(SECONDS_FROM_1900_TO_1970/SECONDS_PER_DAY))

extern struct tm *__brktime( unsigned long, time_t, long, struct tm *);
extern int	  __leapyear( unsigned );
extern int	  __isindst( struct tm * );
extern int	  __getctime( struct tm * );

extern short const __diyr[];	/* days in normal year array */
extern short const __dilyr[];	/* days in leap year array */

static const char *day_name[]
    = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static const char *month_name[]
    = {"January", "February", "March","April","May","June",
       "July", "August",  "September", "October", "November","December"};

const short __month_to_days[2][13] =
    {
    {        /* days in normal year array */
        0,                                      /* Jan */
        31,                                     /* Feb */
        31+28,                                  /* Mar */
        31+28+31,                               /* Apr */
        31+28+31+30,                            /* May */
        31+28+31+30+31,                         /* Jun */
        31+28+31+30+31+30,                      /* Jul */
        31+28+31+30+31+30+31,                   /* Aug */
        31+28+31+30+31+30+31+31,                /* Sep */
        31+28+31+30+31+30+31+31+30,             /* Oct */
        31+28+31+30+31+30+31+31+30+31,          /* Nov */
        31+28+31+30+31+30+31+31+30+31+30,       /* Dec */
        31+28+31+30+31+30+31+31+30+31+30+31     /* Jan, next year */
    },

    {       /* days in leap year array */
        0,                                      /* Jan */
        31,                                     /* Feb */
        31+29,                                  /* Mar */
        31+29+31,                               /* Apr */
        31+29+31+30,                            /* May */
        31+29+31+30+31,                         /* Jun */
        31+29+31+30+31+30,                      /* Jul */
        31+29+31+30+31+30+31,                   /* Aug */
        31+29+31+30+31+30+31+31,                /* Sep */
        31+29+31+30+31+30+31+31+30,             /* Oct */
        31+29+31+30+31+30+31+31+30+31,          /* Nov */
        31+29+31+30+31+30+31+31+30+31+30,       /* Dec */
        31+29+31+30+31+30+31+31+30+31+30+31     /* Jan, next year */
    }
    };

#endif

/* __SRCVERSION("timedata.h $Rev: 153052 $"); */
