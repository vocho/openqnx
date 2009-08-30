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
#include <arm/s3c2400.h>

#define AMPM			0x40	/* clock in 12 hour AM/PM format */
#define MILTIME			0x00	/* clock in 24 hour format */
#define TRANSFER_ENABLE	0x80	/* enable transfer counters -> registers */
#define CMD_REG			11

/*
 * Clock setup for the RTC on the Samsung S3C2400 (ARM 920 core) embedded
 * controller. Registers are defined in s3c2400.h.
 */

int
RTCFUNC(init,s3c2400)(struct chip_loc *chip, char *argv[]) {
    if (chip->phys == NIL_PADDR) {
        chip->phys = S3C2400_RTC_BASE;
    }
    if (chip->access_type == NONE) {
        chip->access_type = MEMMAPPED;
    }
    return S3C2400_RTC_SIZE;
}

int
RTCFUNC(get,s3c2400)(struct tm *tm, int cent_reg) {
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;

/*	
	// convert BCD to binary 
	sec 	= S3C2400_CHIP_READ(S3C2400_BCDSEC);	// seconds
	min 	= S3C2400_CHIP_READ(S3C2400_BCDMIN);	// minutes
	hour	= S3C2400_CHIP_READ(S3C2400_BCDHOUR);	// hours
	mday	= S3C2400_CHIP_READ(S3C2400_BCDDAY);	// day
	mon		= S3C2400_CHIP_READ(S3C2400_BCDMON);	// month
	year	= S3C2400_CHIP_READ(S3C2400_BCDYEAR);	// year
*/

	// convert BCD to binary 
	sec 	= chip_read8(S3C2400_BCDSEC);	// seconds
	min 	= chip_read8(S3C2400_BCDMIN);	// minutes
	hour	= chip_read8(S3C2400_BCDHOUR);	// hours
	mday	= chip_read8(S3C2400_BCDDAY);	// day
	mon		= chip_read8(S3C2400_BCDMON);	// month
	year	= chip_read8(S3C2400_BCDYEAR);	// year

	tm->tm_sec 	= BCD2BIN(sec);
	tm->tm_min 	= BCD2BIN(min);
	tm->tm_hour	= BCD2BIN(hour);
	tm->tm_mday	= BCD2BIN(mday);
	tm->tm_mon	= BCD2BIN(mon) - 1;
	tm->tm_year	= BCD2BIN(year) + 100;

	return(0);
}

int
RTCFUNC(set,s3c2400)(struct tm *tm, int cent_reg) {
	unsigned	seconds;
	unsigned	minutes;
	unsigned	hours;
	unsigned	day;
	unsigned	month;
	unsigned	year;

	// convert binary to BCD
	seconds	= BIN2BCD(tm->tm_sec);
	minutes	= BIN2BCD(tm->tm_min);
	hours	= BIN2BCD(tm->tm_hour);
	day 	= BIN2BCD(tm->tm_mday);
	month	= BIN2BCD(tm->tm_mon + 1);
	year	= BIN2BCD(tm->tm_year % 100);

	chip_write8(S3C2400_BCDSEC, seconds);
	chip_write8(S3C2400_BCDMIN, minutes);
	chip_write8(S3C2400_BCDHOUR, hours | MILTIME);
	chip_write8(S3C2400_BCDDATE, tm->tm_wday + 1);	// 1 to 7, BCD is same as binary, so no conversion needed
	chip_write8(S3C2400_BCDDAY, day);	// day of month
	chip_write8(S3C2400_BCDMON, month);
	chip_write8(S3C2400_BCDYEAR, year);

	return(0);
}
