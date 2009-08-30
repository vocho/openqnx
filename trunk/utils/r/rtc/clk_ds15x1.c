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

#define TRANSFER_ENABLE		(1 << 7)	/* enable transfer counters -> registers */
#define	DS15x1_CONTROLB		0x0F		/* Control B register */

/*
 * Clock setup for DS1501/1511 part.
 * addresses that we use (all fields are BCD)
 *
 * 0 seconds
 * 1 minutes
 * 2 hours
 * 3 day of week (we set this up but do not use it)
 * 4 day
 * 5 month
 * 6 year (0-99)
 * 7 century
 * f control b register
 */

int
RTCFUNC(init,ds15x1)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
		fprintf(stderr,"rtc: -b baseaddr must be specified for ds15x1 clock type\n");
		return (-1);
	}
	if(chip->century_reg == UNSET) 
		chip->century_reg = 0x07;
	if(chip->access_type == NONE) 
		chip->access_type = MEMMAPPED;
	return (16);
}

int
RTCFUNC(get,ds15x1)(struct tm *tm, int cent_reg) {
	unsigned	cent;
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;
	unsigned	ctrlb;

	ctrlb = chip_read8(DS15x1_CONTROLB);
	if (!(ctrlb & TRANSFER_ENABLE)) {
		struct timespec	cur;

		chip_write8(DS15x1_CONTROLB, ctrlb | TRANSFER_ENABLE);
		cur.tv_sec = 0;
		cur.tv_nsec = 10000000;
		nanosleep(&cur, NULL); 	/* wait 0.01 sec for update */
	}

	chip_write8(DS15x1_CONTROLB, ctrlb & ~TRANSFER_ENABLE);
	
	/* convert BCD to binary */
	sec  = chip_read8(0);
	min  = chip_read8(1);
	hour = chip_read8(2);
	mday = chip_read8(4);
	mon  = chip_read8(5) & 0x1F;
	year = chip_read8(6);

	tm->tm_sec 	= BCD2BIN(sec);
	tm->tm_min 	= BCD2BIN(min);
	tm->tm_hour	= BCD2BIN(hour);
	tm->tm_mday	= BCD2BIN(mday);
	tm->tm_mon	= BCD2BIN(mon) - 1;
	tm->tm_year	= BCD2BIN(year);
	if (cent_reg >= 0) {
		cent	= chip_read8(cent_reg);	/* century */
		if (cent == 0x20)
			tm->tm_year += 100;
	} else if (tm->tm_year < 70)
		tm->tm_year += 100;

	chip_write8(DS15x1_CONTROLB, ctrlb | TRANSFER_ENABLE);

#ifdef DIAG
	fprintf(stderr,"rtc read: cent=%d; year=%d\n",cent,tm->tm_year);
#endif

	return(0);
}

int
RTCFUNC(set,ds15x1)(struct tm *tm, int cent_reg) {
	unsigned	seconds;
	unsigned	minutes;
	unsigned	hours;
	unsigned	day;
	unsigned	month;
	unsigned	year;
	unsigned	cent;
	unsigned	ctrlb;

	/* convert binary to BCD */
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

	month |= chip_read8(5) & 0xE0;	/* EOSC#/E32K#/BB32 */

	ctrlb = chip_read8(DS15x1_CONTROLB);
	chip_write8(DS15x1_CONTROLB, ctrlb & ~TRANSFER_ENABLE);

	chip_write8(0, seconds);
	chip_write8(1, minutes);
	chip_write8(2, hours);
	chip_write8(3, tm->tm_wday + 1);
	chip_write8(4, day);
	chip_write8(5, month);
	chip_write8(6, year);
	if (cent_reg >= 0)
		chip_write8(cent_reg, cent);

	chip_write8(DS15x1_CONTROLB, ctrlb | TRANSFER_ENABLE);

	return (0);
}
