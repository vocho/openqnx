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
 * Clock setup for M48T5x part. This is an embedded part so we can use
 * what ever conventions we want for the century byte.
 */

static	unsigned
rdcmos(unsigned i) {
	unsigned temp;

	_disable();		
	chip_write8(0, i & 0xff);			// Low 8 bits of addr
	chip_write8(1, (i >> 8) & 0xff);	// High 8 bits
 	temp = chip_read8(3);				// Read data
	_enable();		
	return(temp);
}


static void
wrcmos(unsigned i, unsigned d) {
	_disable();
	chip_write8(0,i);
	chip_write8(1,d);
	chip_write8(0, i & 0xff);			// Low 8 bits of addr
	chip_write8(1, (i >> 8) & 0xff);	// High 8 bits
 	chip_write8(3, d);					// Write data
	_enable();	
}

#define M48T5X_TIME_REGS 0x1ff0

int
RTCFUNC(init,m48t5x)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
		fprintf(stderr,"rtc: -b baseaddr must be specified for m48t5x clock type\n");
		return(-1);
	}
	if(chip->century_reg == UNSET) 
			chip->century_reg = -1;
	if(chip->access_type == NONE) 
			chip->access_type = MEMMAPPED;
	return(4);
}

int
RTCFUNC(get,m48t5x)(struct tm *tm, int cent_reg) {
	unsigned	cent;
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;

	do {
		sec  = rdcmos(M48T5X_TIME_REGS + 0x9);
		min  = rdcmos(M48T5X_TIME_REGS + 0xa);
		hour = rdcmos(M48T5X_TIME_REGS + 0xb);
		mday = rdcmos(M48T5X_TIME_REGS + 0xd);
		mon  = rdcmos(M48T5X_TIME_REGS + 0xe);
		year = rdcmos(M48T5X_TIME_REGS + 0xf);

		//Loop while time inconsistent
	} while(sec != rdcmos(M48T5X_TIME_REGS + 0x9));

	tm->tm_sec  = BCD2BIN(sec & ~0x80);
	tm->tm_min  = BCD2BIN(min);
	tm->tm_hour = BCD2BIN(hour);
	tm->tm_mday = BCD2BIN(mday);
	tm->tm_mon  = BCD2BIN(mon) - 1;
	tm->tm_year = BCD2BIN(year);

	if(cent_reg >= 0) {
		cent	= rdcmos(cent_reg);	// century
		cent	= BCD2BIN(cent);
		if(cent == 20) tm->tm_year += 100;
	} else if(tm->tm_year < 70) {
		tm->tm_year += 100;
	}

	return(0);
}

int
RTCFUNC(set,m48t5x)(struct tm *tm, int cent_reg) {
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;
	unsigned	cent;
	unsigned	control;

	// convert binary to BCD
	sec		= BIN2BCD(tm->tm_sec);
	min		= BIN2BCD(tm->tm_min);
	hour	= BIN2BCD(tm->tm_hour);
	mday 	= BIN2BCD(tm->tm_mday);
	mon		= BIN2BCD(tm->tm_mon + 1);
	year	= BIN2BCD(tm->tm_year % 100);
	cent	= BIN2BCD((tm->tm_year / 100) + 19);

	control = rdcmos(M48T5X_TIME_REGS+0x8);

	//Stop the clock
	wrcmos(M48T5X_TIME_REGS+0x8, control | 0x80);

	wrcmos(M48T5X_TIME_REGS + 0x9, sec);
	wrcmos(M48T5X_TIME_REGS + 0xa, min);
	wrcmos(M48T5X_TIME_REGS + 0xb, hour);
	wrcmos(M48T5X_TIME_REGS + 0xd, mday);
	wrcmos(M48T5X_TIME_REGS + 0xe, mon);
	wrcmos(M48T5X_TIME_REGS + 0xf, year);

	if(cent_reg >= 0) {
		wrcmos(cent_reg, cent);
	}

	//Restart the clock
	wrcmos(M48T5X_TIME_REGS+0x8, control & ~0x80);

	return(0);
}
