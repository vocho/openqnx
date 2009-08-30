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

/*
 * Clock setup for DS1743 part.

 * addresses that we use (all fields are BCD)

 0 - 0x1ff7		NVRAM
 0x1ff8			control & century bits (0-39)
 0x1ff9			OSC & seconds (0-59)
 0x1ffa			minute (0-59)
 0x1ffb			hour (0-23)
 0x1ffc			day of week (1-7)
 0x1ffd			date (1-31)
 0x1ffe			month (1-12)
 0x1fff			year (0-99)
*/

#define DS1743_CONTROL		0x1ff8
#define DS1743_SECONDS		0x1ff9
#define DS1743_MINUTES		0x1ffa
#define DS1743_HOUR			0x1ffb
#define DS1743_DAY			0x1ffc
#define DS1743_DATE			0x1ffd
#define DS1743_MONTH		0x1ffe
#define DS1743_YEAR			0x1fff

#define DS1743_CONTROL_R			0x40
#define DS1743_CONTROL_W			0x80
#define DS1743_CONTROL_CENT_MASK	0x3f

#define DS1743_SECONDS_OSC			0x80
#define DS1743_SECONDS_MASK			0x7f

#define DS1743_DAY_FT				0x40
#define DS1743_DAY_BF				0x40
#define DS1743_DAY_MASK				0x07

#define DS1743_MINUTES_MASK			0x7f
#define DS1743_HOUR_MASK			0x3f
#define DS1743_DATE_MASK			0x3f
#define DS1743_MONTH_MASK			0x1f

int
RTCFUNC(init,ds1743)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
		fprintf(stderr,"rtc: -b baseaddr must be specified for ds1743 clock type\n");
		return(-1);
	}
	if(chip->access_type == NONE) 
			chip->access_type = MEMMAPPED;
	return(DS1743_YEAR+1);
}

int
RTCFUNC(get,ds1743)(struct tm *tm, int cent_reg) {
	unsigned	cent;
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;
	unsigned	reg;
	
	// Stop the chip from updating
	chip_write8(DS1743_CONTROL, chip_read8(DS1743_CONTROL) | DS1743_CONTROL_R);

	reg = chip_read8(DS1743_SECONDS);
	if(reg & DS1743_SECONDS_OSC) {
		// clock oscillator not running
		chip_write8(DS1743_SECONDS, reg & ~DS1743_SECONDS_OSC);
	}
	reg = chip_read8(DS1743_DAY);
	if(reg & DS1743_DAY_FT) {
		// need to turn off frequency test mode
		chip_write8(DS1743_DAY, reg & ~DS1743_DAY_FT);
	}

	// convert BCD to binary 
	sec 	= chip_read8(DS1743_SECONDS) & DS1743_SECONDS_MASK;
	min 	= chip_read8(DS1743_MINUTES) & DS1743_MINUTES_MASK;
	hour	= chip_read8(DS1743_HOUR) & DS1743_HOUR_MASK;
	mday	= chip_read8(DS1743_DATE) & DS1743_DATE_MASK;
	mon		= (chip_read8(DS1743_MONTH) & DS1743_MONTH_MASK) - 1;
	year	= chip_read8(DS1743_YEAR);
	cent	= chip_read8(DS1743_CONTROL) & DS1743_CONTROL_CENT_MASK;

	// Start the chip updating again
	chip_write8(DS1743_CONTROL, chip_read8(DS1743_CONTROL) & ~DS1743_CONTROL_R);

	tm->tm_sec 	= BCD2BIN(sec);
	tm->tm_min 	= BCD2BIN(min);
	tm->tm_hour	= BCD2BIN(hour);
	tm->tm_mday	= BCD2BIN(mday);
	tm->tm_mon	= BCD2BIN(mon);
	tm->tm_year	= BCD2BIN(year) + (BCD2BIN(cent)-19) * 100;

	return(0);
}

int
RTCFUNC(set,ds1743)(struct tm *tm, int cent_reg) {
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

	// Prep for updating
	chip_write8(DS1743_CONTROL, chip_read8(DS1743_CONTROL) | DS1743_CONTROL_W);

//printf("s:%x m:%x h:%x d:%x M:%x y:%x c:%x\n", seconds, minutes, hours, day, month, year, cent);
	chip_write8(DS1743_SECONDS, seconds);
	chip_write8(DS1743_MINUTES, minutes);
	chip_write8(DS1743_HOUR, hours);
	chip_write8(DS1743_DATE, day);
	chip_write8(DS1743_MONTH, month);
	chip_write8(DS1743_YEAR, year);
	chip_write8(DS1743_CONTROL, cent);

	// Indicate end of updating
	chip_write8(DS1743_CONTROL, chip_read8(DS1743_CONTROL) &  ~DS1743_CONTROL_W);

	return(0);
}
