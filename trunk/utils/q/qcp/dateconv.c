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





#include <stdio.h>
#include <time.h>

static long year_sec[15] = {
	31622400L, 63158400L, 94694400L, 126230400L, 157852800L, 189388800L,
	220924800L, 252460800L, 284083200L, 315619200L, 347155200L, 378691200L,
	410313600L, 441849600L, 473385600L 
	} ;

static long month_sec[11] = {
	2678400L, 5097600L, 7776000L, 10368000L, 13046400L, 15638400L,
	18316800L, 20995200L, 23587200L, 26265600L, 28857600L 
	} ;

static long sec_per_day		= 86400L;

long odtond(date)
unsigned short date[2];
	{
	register unsigned short *dp = date;
	unsigned short day, month, year;
	long seconds = 0;

	if((dp[0] & 0x0f00) == 0  ||  (dp[0] & 0x3e) == 0)
		return(0);

	year = dp[0] >> 12;
	month = (dp[0] >> 8) - 1 & 0x000f;
	day = (dp[0] >> 1) - 1 & 0x1f;

	if(year)
		seconds = year_sec[year - 1];

	if(month)
		seconds += month_sec[month - 1];

	if((dp[0] & 0x3000) == 0  &&  month > 1)
		++day;

	seconds += day * sec_per_day;

	if(dp[0] & 0x0001)
		seconds += 43200u;

	seconds += dp[1];

	return(seconds);
	}


void ndtood(seconds, date)
long seconds;
unsigned short date[];
	{
	unsigned short sec, day, month, year;
	long leap_day = 0;
	register unsigned short *dp = date;

	for(year = 0 ; year < 15 ; ++year)
		if(seconds < year_sec[year])
			break;

	if(year)
		seconds -= year_sec[year - 1];

	if((year & 0x03) == 0) {
		leap_day = 43200u;
		leap_day += 43200u;
		}

	for(month = 0 ; month < 11 ; ++month)
		if(seconds < month_sec[month] + (month ? leap_day : 0))
			break;

	if(month) {
		seconds -= month_sec[month - 1];
		if(month > 1)
			seconds -= leap_day;
		}

	day = seconds / 43200;
	sec = seconds % 43200;
#ifdef NEVER
	asm("	mov ax,14[bp]");
	asm("	mov dx,16[bp]");
	asm("	and dx,#7fffh");		/* To prevent overflow in DIV */
	asm("	mov cx,#43200");
	asm("	div cx");
	asm("	mov -4[bp],ax");		/* day <- ax */
	asm("	mov -2[bp],dx");		/* sec <- dx */
#endif

	year <<= 12;
	month <<= 8;

	dp[0] = (day + 0x0002) | (month + 0x0100) | year;
	dp[1] = sec;
	}
