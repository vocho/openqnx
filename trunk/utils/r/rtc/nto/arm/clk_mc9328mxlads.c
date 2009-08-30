/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */

#include "rtc.h"
#include <time.h>
#include <arm/mx1.h>

/*
 * RTC year definition (There are no years in rtc module. So we hardwire current year to 2007)
 */
#define MX1_CURRENT_YEAR 2007
#define MX1_DAYS_OF_YEAR 365

static int daypermonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

/* Convert day since Jan 1 to month since Jan and day of the month
 * yearday: 0 - 364
 * month: 0 - 11
 * day of the month: 1 - xx
 */
int 
yearday2monthday(int yearday, int * monthday, int * month)
{
	/* Decide if it is a leap year */
	if (MX1_DAYS_OF_YEAR!=365) daypermonth[1] = 29;

	*month = 0;
	if ((yearday<0)||(yearday>MX1_DAYS_OF_YEAR-1)) {
#ifdef	VERBOSE_SUPPORTED
		if (verbose) {
			printf("days of the year out of range, forced to Janury 1st. \n");
		}
#endif
		*monthday = 1;
		*month = 0;
                return 0;
	}

	while (yearday>=daypermonth[*month]) 
	        yearday -= daypermonth[(*month)++];

        *monthday = yearday+1;

        return 1;
}

/* Convert day since Jan 1 to month since Jan and day of the month
 * yearday: 0 - 364
 * month: 0 - 11
 * day of the month: 1 - xx
 */
int 
monthday2yearday(int month, int monthday, int * yearday)
{
	int i = 0;

	/* Decide if it is a leap year */
	if (MX1_DAYS_OF_YEAR!=365) daypermonth[1] = 29;
	
	if ((month<0)||(month>11)) {
		month = 0;
#ifdef	VERBOSE_SUPPORTED
		if (verbose) {
			printf("month out of range, forced to Janury. \n");
		}
#endif
	}
	if ((monthday<1)||(monthday>=daypermonth[month])) monthday = 1;

	*yearday = 0;
	while (i < month ) *yearday += daypermonth[i++];
	*yearday += monthday - 1;

	return 1;
}

int
RTCFUNC(init,mc9328mxlads)(struct chip_loc *chip, char *argv[])
{

	if (chip->phys == NIL_PADDR) {
		chip->phys = MX1_RTC_BASE;
	}
	if (chip->access_type == NONE) {
		chip->access_type = IOMAPPED;
	}

	return MX1_RTC_SIZE;
}

int
RTCFUNC(get,mc9328mxlads)(struct tm *tm, int cent_reg)
{
	unsigned ydays = 0, months = 0, mdays = 0, hours = 0, minutes = 0, seconds = 0;
	time_t systemt;

	/*
	 * read RTC counter value
	 */
	ydays = chip_read(MX1_RTC_DAYR, 32) & 0x1FF;
	hours = (chip_read(MX1_RTC_HOURMIN, 32) >> 8 ) & 0x1F;
	minutes = chip_read(MX1_RTC_HOURMIN, 32) & 0x3F;
	seconds = chip_read(MX1_RTC_SECONDS, 32) & 0x3F;
	
#ifdef	VERBOSE_SUPPORTED
	if (verbose) {
		printf("\nCurrent RTC year: %d\n", MX1_CURRENT_YEAR);
		printf("RTC read: days of the year=%d hours=%d minutes=%d seconds=%d \n", ydays, hours, minutes, seconds);
	}
#endif
	
	systemt = time(NULL);
	
	gmtime_r(&systemt, tm);

#ifdef	VERBOSE_SUPPORTED
	if (verbose) {
		printf("month=%d month day=%d hour=%d min=%d sec=%d \n", tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
#endif
	
	if (yearday2monthday(ydays, (int *)&mdays, (int *)&months)==0) {
		ydays = 0;
                chip_write(MX1_RTC_DAYR, 0x0, 32);
#ifdef	VERBOSE_SUPPORTED
		if (verbose) {
			printf("Days of year is out of range.\n");
			printf("Force to January 1st: ydays=%d hours=%d minutes=%d seconds=%d \n", ydays, hours, minutes, seconds);
		}
#endif
        }
	
	tm->tm_sec = seconds;
	tm->tm_min = minutes;
	tm->tm_hour = hours;
	tm->tm_mday = mdays;
	tm->tm_mon = months;
	tm->tm_wday = 0;
	tm->tm_yday = 0;
	
	return 0;
}

int
RTCFUNC(set,mc9328mxlads)(struct tm *tm, int cent_reg)
{
	unsigned int yearday;
	
	monthday2yearday(tm->tm_mon, tm->tm_mday, (int *)&yearday);
	
#ifdef	VERBOSE_SUPPORTED
	if (verbose) {
		printf("\nCurrent RTC year: %d\n", MX1_CURRENT_YEAR);
		printf("RTC write: days of the year=%d month=%d days=%d hours=%d minutes=%d seconds=%d Register MX1_RTC_HOURMIN=%d\n", yearday, tm->tm_mon, tm->tm_mday, (tm->tm_hour)&0x1F, (tm->tm_min)&0x3F, tm->tm_sec&0x3F, (((tm->tm_hour)&0x1F)<<8) + ((tm->tm_min)&0x3F) );
	}
#endif
	
	chip_write(MX1_RTC_DAYR, yearday&0x1FF, 32);
	chip_write(MX1_RTC_HOURMIN, (((tm->tm_hour)&0x1F)<<8) + ((tm->tm_min)&0x3F), 32);
	chip_write(MX1_RTC_SECONDS, tm->tm_sec&0x3F, 32);

	return 0;
}
