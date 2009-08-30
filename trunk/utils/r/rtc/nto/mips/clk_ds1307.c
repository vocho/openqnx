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

#define AMPM			0x40	/* clock in 12 hour AM/PM format */
#define MILTIME			0x00	/* clock in 24 hour format */
#define CLK_HALT		0x80	/* disable the oscillator (in the seconds reg) */
#define CMD_REG			0x07
#define SQWE			0x10	/* enable square wave output (CMD_REG) */

/*
 * Clock setup for DS1307 part. This is an embedded part so we can use
 * what ever conventions we want for the century byte.

 * addresses that we use (all fields are BCD)

 0 seconds
 1 minutes
 2 hours & hour format
 3 day of week (we set this up but do not use it)
 4 day
 5 month
 6 year (0-99)
 7 command register 

 8 - 3F user area 56 bytes battery backed up SRAM
 8 user area 1 (century) [default]
 9 user area 2 (OS reserved)

 */
int
RTCFUNC(init,ds1307)(struct chip_loc *chip, char *argv[]) {
	if(chip->resmgr_path[0] == '\0') {
		strcpy(chip->resmgr_path, "/dev/_2wire_1");
//		fprintf(stderr,"rtc: -p resmgr_path must be specified for ds1307 clock type\n");
//		return(-1);
	}
	if(chip->century_reg == UNSET) 
			chip->century_reg = 0x08;
	if(chip->access_type == NONE) 
			chip->access_type = RESMGR;
	
	chip->dev_write_addr = 0xD0;
	chip->dev_read_addr = 0xD1;
	return(16);
}

int
RTCFUNC(get,ds1307)(struct tm *tm, int cent_reg) {
	unsigned	cent;
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;
	

	if(chip_read8(CMD_REG) & CLK_HALT) {
		struct timespec	cur;

		chip_write8(0, chip_read8(0) & ~CLK_HALT);
		cur.tv_sec = 0;
		cur.tv_nsec = 10000000;
		nanosleep(&cur, NULL); 	/* wait 0.01 sec for update */
	}

	// convert BCD to binary 
	sec 	= chip_read8(0) & 0x3f;		// seconds
	min 	= chip_read8(1) & 0x7f;		// minutes
	hour	= chip_read8(2) & 0x3f;		// hours
	mday	= chip_read8(4) & 0x3f;		// day
	mon	= chip_read8(5) & 0x1f;		// month
	year	= chip_read8(6);		// year

	tm->tm_sec 	= BCD2BIN(sec);
	tm->tm_min 	= BCD2BIN(min);
	tm->tm_hour	= BCD2BIN(hour);
	tm->tm_mday	= BCD2BIN(mday);
	tm->tm_mon	= BCD2BIN(mon) - 1;
	tm->tm_year	= BCD2BIN(year);
	if(cent_reg >= 0) {
		cent	= chip_read8(cent_reg);	// century
		cent	= BCD2BIN(cent);
		if(cent == 20) tm->tm_year += 100;
	} else if(tm->tm_year < 70) {
		tm->tm_year += 100;
	}

#ifdef DIAG
	fprintf(stderr,"rtc read: cent=0x%.2x; year=%d\n",cent,tm->tm_year);
#endif

	return(0);
}

int
RTCFUNC(set,ds1307)(struct tm *tm, int cent_reg) {
	unsigned	seconds;
	unsigned	minutes;
	unsigned	hours;
	unsigned	day;
	unsigned	month;
	unsigned	year;
	unsigned	cent;

	// convert binary to BCD
	seconds	= BIN2BCD(tm->tm_sec);
	minutes	= BIN2BCD(tm->tm_min);
	hours	= BIN2BCD(tm->tm_hour);
	day 	= BIN2BCD(tm->tm_mday);
	month	= BIN2BCD(tm->tm_mon + 1);
	year	= BIN2BCD(tm->tm_year % 100);
	cent	= BIN2BCD((tm->tm_year / 100) + 19);

#ifdef DIAG
	fprintf(stderr,"rtc set: cent=0x%.2x; year=0x%.2x (after adjustment)\n",cent,year);
#endif

	chip_write8(CMD_REG, chip_read8(CMD_REG) & ~SQWE);	/* disable square wave output */

	chip_write8(0, seconds);
	chip_write8(1, minutes);
	chip_write8(2, hours | MILTIME);
	chip_write8(3, tm->tm_wday + 1);
	chip_write8(4, day);
	chip_write8(5, month);
	chip_write8(6, year);
	if(cent_reg >= 0) {
		chip_write8(cent_reg, cent);
	}

	chip_write8(CMD_REG, chip_read8(CMD_REG) | SQWE);	/* re-enable square wave output */	

	return(0);
}
