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
#include <arm/tuareg.h>

/*
 * Clock setup for the RTC on the TUAREG (ARM 720T core).
 * Registers are defined in tuareg.h
 */

int
RTCFUNC(init,tuareg)(struct chip_loc *chip, char *argv[]) {
    if (chip->phys == NIL_PADDR) {
        chip->phys = TUAREG_RTC_BASE;
    }
    if (chip->access_type == NONE) {
        chip->access_type = MEMMAPPED;
    }
    return TUAREG_RTC_SIZE;
}

int
RTCFUNC(get,tuareg)(struct tm *tm, int cent_reg) {
	time_t				t;
	uint_t		rtc_tick_l,
				rtc_tick_m,
				rtc_tick_h;


	/*
	 * read RTC counter value
	 */

	rtc_tick_l = chip_read(TUAREG_RTC_RTCCNT_L,16);
	rtc_tick_m = chip_read(TUAREG_RTC_RTCCNT_M,16);
	rtc_tick_h = chip_read(TUAREG_RTC_RTCCNT_H,16);

	t = (rtc_tick_l/32768) + (rtc_tick_m * 2) + (rtc_tick_h * 0x20000);

#ifdef	VERBOSE_SUPPORTED
	if (verbose) {
		printf("rtc read: %x\n", t);
	}
#endif

	gmtime_r(&t,tm);	

	return(0);
}

int
RTCFUNC(set,tuareg)(struct tm *tm, int cent_reg) {
	time_t		t;
	
	t = mktime(tm);

	/*
	 *	mktime assumes local time.  We will subtract timezone
	 */
	t -= timezone;

#ifdef	VERBOSE_SUPPORTED
	if (verbose) {
		printf("rtc write: %x\n", t);
	}
#endif

	/* Enter configuration mode			*/
	
	chip_write(TUAREG_RTC_RTCCR, TUAREG_RTCCR_CNF, 16);
	
    /*	Make sure we can write to the RTC registers	*/
	while(!(chip_read(TUAREG_RTC_RTCCR,16) & TUAREG_RTCCR_RTOFF));
	chip_write(TUAREG_RTC_RTCCNT_H, t/0x20000,16);

	while(!(chip_read(TUAREG_RTC_RTCCR,16) & TUAREG_RTCCR_RTOFF));
	chip_write(TUAREG_RTC_RTCCNT_M,(t%0x20000)/2, 16);

	while(!(chip_read(TUAREG_RTC_RTCCR,16) & TUAREG_RTCCR_RTOFF));
	chip_write(TUAREG_RTC_RTCCNT_L, ((t/0x20000)/2)%2, 16);

	while(!(chip_read(TUAREG_RTC_RTCCR,16) & TUAREG_RTCCR_RTOFF));

	/*	Leave configution mode		*/	
	chip_write(TUAREG_RTC_RTCCR, (chip_read(TUAREG_RTC_RTCCR,16) & ~TUAREG_RTCCR_CNF),16);
	return(0);
}
