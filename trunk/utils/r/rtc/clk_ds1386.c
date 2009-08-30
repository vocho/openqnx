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
#define TRANSFER_ENABLE	0x80	/* enable transfer counters -> registers */
#define CMD_REG			11

/*
 * Clock setup for DS1386 part. This is an embedded part so we can use
 * what ever conventions we want for the century byte.

 * addresses that we use (all fields are BCD)

 0 hundreths of seconds
 1 seconds
 2 minutes
 4 hours & hour format
 6 day of week (we set this up but do not use it)
 8 day
 9 month
 A year (0-99)
 B command register 
 E user area 1 (century) [default]
 F user area 2 (OS reserved)
 10 - 7FFF (7FFF) user area 32K (8K) battery backed up SRAM

 */
int
RTCFUNC(init,ds1386)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
		fprintf(stderr,"rtc: -b baseaddr must be specified for ds1386 clock type\n");
		return(-1);
	}
	if(chip->century_reg == UNSET) 
			chip->century_reg = 0x0e;
	if(chip->access_type == NONE) 
			chip->access_type = MEMMAPPED;
	return(16);
}

int
RTCFUNC(get,ds1386)(struct tm *tm, int cent_reg) {
	unsigned	cent;
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;
	

	if(!(chip_read8(CMD_REG) & TRANSFER_ENABLE)) {
		struct timespec	cur;

		chip_write8(CMD_REG, chip_read8(CMD_REG) | TRANSFER_ENABLE);
		cur.tv_sec = 0;
		cur.tv_nsec = 10000000;
		nanosleep(&cur, NULL); 	/* wait 0.01 sec for update */
	}

	chip_write8(CMD_REG, chip_read8(CMD_REG) & ~TRANSFER_ENABLE);
	
	// convert BCD to binary 
	sec 	= chip_read8(1);		// seconds
	min 	= chip_read8(2);		// minutes
	hour	= chip_read8(4) & 0x3f;	// hours
	mday	= chip_read8(8);		// day
	mon		= chip_read8(9) & 0x3f;	// month
	year	= chip_read8(10);		// year

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

	chip_write8(CMD_REG, chip_read8(CMD_REG) | TRANSFER_ENABLE);

#ifdef DIAG
	fprintf(stderr,"rtc read: cent=%d; year=%d\n",cent,tm->tm_year);
#endif

	return(0);
}

int
RTCFUNC(set,ds1386)(struct tm *tm, int cent_reg) {
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
	fprintf(stderr,"rtc set: cent=%d; year=%d (after adjustment)\n",cent,year);
#endif

	month |= chip_read8(9) & 0x40;	/* ESQW# */

	chip_write8(9, 0);	/* turn off clock */

	chip_write8(0, 0);	//hundredth's of second
	chip_write8(1, seconds);
	chip_write8(2, minutes);
	chip_write8(4, hours | MILTIME);
	chip_write8(6, tm->tm_wday + 1);
	chip_write8(8, day);
	chip_write8(10, year);
	if(cent_reg >= 0) {
		chip_write8(cent_reg, cent);
	}

	chip_write8(9, month & 0x5f);	/* EOSC# low to start clock */
	chip_write8(CMD_REG, chip_read8(CMD_REG) | TRANSFER_ENABLE);

	return(0);
}
