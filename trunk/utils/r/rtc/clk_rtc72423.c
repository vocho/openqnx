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





/*
 * This is the version which deals with the FOX RTC-72423 part. 
 */
#include "rtc.h"

/* register D */
#define SECADJ			0x80	/* 30 sec adj clock */
#define IRQFLAG			0x40	/* IRQ reset/flag */
#define BUSY			0x20	/* clock in transferring data */
#define HOLD			0x10	/* stop clock transfer */

/* register E */
#define T0				0x80	/* interrupt source */
#define T1				0x40	/* interrupt source */
#define ITRPT			0x20	/* interrupt mode */
#define MASK			0x10	/* interrupt mask */

/* register F */
#define TEST			0x80	/* 30 sec adj clock */
#define AMPM			0x40	/* clock in 12 hour AM/PM format */
#define STOP			0x20	/* stop the clock divider */
#define RESET			0x10	/* reset the clock chip register */

#define REG_D			0xd
#define REG_E			0xe
#define REG_F			0xf

/*
 * Clock setup for FOX RTC-72423 part. This is an embedded part so we can use
 * what ever conventions we want for the year registers. Note there is no
 * Century byte or other ram on this so we use the year between 0-70 to
 * indicate if it is the next century/
 *
 * addresses that we use (all fields are BCD)

 0 seconds 1
 1 seconds 10
 2 minutes 1
 3 minutes 10
 4 hours 1 
 5 hours 10 & hour format
 6 day 1
 7 day 10
 8 month 1
 9 month 10
 A year 1
 B year 10
 C day of week (we set this up but do not use it)
 D command register 
 E interrup control
 F command register 

 */

int
RTCFUNC(init,rtc72423)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
		fprintf(stderr,"rtc: -b baseaddr must be specified for 72423 clock type\n");
		return(-1);
	}
	if(chip->access_type == NONE) 
			chip->access_type = IOMAPPED;
	return(16);
}

int
RTCFUNC(get,rtc72423)(struct tm *tm, int cent_reg) {
	unsigned	seconds;
	unsigned	minutes;
	unsigned	hours;
	unsigned	day;
	unsigned	month;
	unsigned	year;

	chip_write8(REG_D, HOLD);			/* stop the clock */

	while(chip_read8(REG_D) & BUSY) {
		struct timespec cur;

		chip_write8(REG_D, 0);				/* HOLD=0 */
		cur.tv_sec = 0;
		cur.tv_nsec = 190000;
		nanosleep(&cur, NULL); 	/* wait 190 usec for update */
		chip_write8(REG_D, HOLD);			/* stop the clock */
	}

	// get the data
	seconds	= (chip_read8(1)  <<4) | chip_read8(0);
	minutes	= (chip_read8(3)  <<4) | chip_read8(2);
	hours	= ((chip_read8(5) & 3)  <<4) | chip_read8(4);
	day		= (chip_read8(7)  <<4) | chip_read8(6);
	month	= (chip_read8(9)  <<4) | chip_read8(8);
	year	= (chip_read8(11) <<4) | chip_read8(10);

	chip_write8(REG_D, 0);				/* HOLD=0 */

	// convert BCD to binary 
	tm->tm_sec	= BCD2BIN(seconds);
	tm->tm_min	= BCD2BIN(minutes);
	tm->tm_hour	= BCD2BIN(hours);
	tm->tm_mday	= BCD2BIN(day);
	tm->tm_mon	= BCD2BIN(month) - 1;
	tm->tm_year	= BCD2BIN(year);
	if(tm->tm_year < 70) tm->tm_year += 100;

	return(0);
}

int
RTCFUNC(set,rtc72423)(struct tm *tm, int cent_reg) {
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

	// we assume the chip has never been setup.
	chip_write8(REG_F, RESET | AMPM | STOP); /* test=0 */

	chip_write8(0, seconds);
	chip_write8(1, seconds>>4);
	chip_write8(2, minutes);
	chip_write8(3, minutes>>4);
	chip_write8(4, hours);
	chip_write8(5, hours>>4);
	chip_write8(6, day);
	chip_write8(7, day>>4);
	chip_write8(8, month);
	chip_write8(9, month>>4);
	chip_write8(10, year);
	chip_write8(11, year>>4);

	chip_write8(REG_E, 0);				/* Enable signal out @ 1/64 sec */

	chip_write8(REG_D, 0);				/* 30sec=0, IRQ=0, HOLD=0 */

	chip_write8(REG_F, AMPM);			/* start clock. TEST=0, STOP=0, RESET=0 */

	return(0);
}
