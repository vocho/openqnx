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
#include <arm/pxa250.h>


int
RTCFUNC(init,pxa250)(struct chip_loc *chip, char *argv[])
{
	if (chip->phys == NIL_PADDR) {
		chip->phys = PXA250_RTC_BASE;
	}
	if (chip->access_type == NONE) {
		chip->access_type = MEMMAPPED;
	}
	return PXA250_RTC_SIZE;
}

int
RTCFUNC(get,pxa250)(struct tm *tm, int cent_reg)
{
	time_t		t;

	/*
	 * read RTC counter value
	 */
	t = chip_read(PXA250_RCNR, 32);

#ifdef	VERBOSE_SUPPORTED
	if (verbose) {
		printf("rtc read: %d\n", t);
	}
#endif
	
	gmtime_r(&t,tm);	
	
	return 0;
}

int
RTCFUNC(set,pxa250)(struct tm *tm, int cent_reg)
{
	time_t		t;
	
	t = mktime(tm);

	/*
	 *	mktime assumes local time.  We will subtract timezone
	 */
	t -= timezone;

#ifdef	VERBOSE_SUPPORTED
	if (verbose) {
		printf("rtc write: %d\n", t);
	}
#endif

	chip_write(PXA250_RCNR, t, 32);

	return 0;
}
