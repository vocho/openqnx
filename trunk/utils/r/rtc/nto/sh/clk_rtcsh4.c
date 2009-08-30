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
#include <sh/cpu.h>
#include <sh/rtc.h>

/*
 * Clock setup for SH4 onchip RTC.
 * addresses that we use (all fields are BCD) Need Shift

 0 1/128 seconds
 1 seconds
 2 minutes
 3 hours & hour format
 4 day of week (we set this up but do not use it)
 5 day
 6 month
 7 year (0000-9999)
 8 Second Alarm
 9 Minute Alarm
 10 Hour Alarm
 11 Day of the Week Alarm
 12 Day Alarm
 13 Month Alarm
 14 control register 1
 15 control register 2
*/

int
RTCFUNC(init,rtcsh4)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
		chip->phys = P4_TO_A7(SH_MMR_RTC_R64CNT);
	}
	if(chip->access_type == NONE) 
			chip->access_type = IOMAPPED;
	return(64);
}

int
RTCFUNC(get,rtcsh4)(struct tm *tm, int cent_reg) {

	// read RTC value
   	do {
		// clear carry bit
		chip_write8(14,0);

		// convert BCD to binary 
		tm->tm_sec 	= BCD2BIN(chip_read8(1));		// seconds
		tm->tm_min 	= BCD2BIN(chip_read8(2));		// minutes
		tm->tm_hour	= BCD2BIN(chip_read8(3));		// hours
		tm->tm_mday	= BCD2BIN(chip_read8(5));		// day
		tm->tm_mon	= BCD2BIN(chip_read8(6)) - 1;		// month
		tm->tm_year	= BCD2BIN(chip_read16(7));		// year

		//Loop while time inconsistent
	} while(chip_read8(14) & SH_RTC_RCR1_CF);

	tm->tm_year -= 1900;

	return(0);
}

int
RTCFUNC(set,rtcsh4)(struct tm *tm, int cent_reg) {
	
	// Stop clock, reset frequency
	chip_write8(15, SH_RTC_RCR1_RESET | SH_RTC_RCR1_RTCEN);

	// set time
	chip_write8(1, BIN2BCD(tm->tm_sec));
	chip_write8(2, BIN2BCD(tm->tm_min));
	chip_write8(3, BIN2BCD(tm->tm_hour));
	chip_write8(5, BIN2BCD(tm->tm_mday));
	chip_write8(6, BIN2BCD(tm->tm_mon + 1));
	chip_write16(7, BIN2BCD(tm->tm_year + 1900));

	// start clock
	chip_write8(15, SH_RTC_RCR1_START | SH_RTC_RCR1_RTCEN);


	return(0);
}
