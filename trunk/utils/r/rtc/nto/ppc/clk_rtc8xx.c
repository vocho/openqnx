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
#include <ppc/800cpu.h>


#define get_spr(spr)	({unsigned __val; __asm__ __volatile__( "mfspr %0,%1" : "=r" (__val) : "i" (spr) ); __val; })


extern struct chip_loc	chip;
union {
	volatile uint8_t	*p;
	unsigned			port;
}			chip_base;


int
RTCFUNC(init,rtc8xx)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
		chip->phys = (get_spr(PPC800_SPR_IMMR)&0xffff0000) + PPC800_IMMR_RTCSC;
	}
	if(chip->century_reg == UNSET) 
			chip->century_reg = 0x0e;
	if(chip->access_type == NONE) 
			chip->access_type = MEMMAPPED;
	return(64);
}

int
RTCFUNC(get,rtc8xx)(struct tm *tm, int cent_reg) {
	time_t		t;

	//
	//	make sure RTC is on
	//
	
	if(!(*(chip_base.p+1) & 0x01)) 
		// turn on RTC in RTCSC
	   	*(chip_base.p+1) |= 0x01;

#ifdef DIAG
	fprintf(stderr,"RTCSC		%x\n",*(short int *)chip_base.p);
#endif
	
	// read RTC value

	t=*(long *)(chip_base.p+4);
	
#ifdef DIAG
	printf(stderr,"RTC		%x\n",*(int *)(chip_base.p+4));
#endif

	gmtime_r(&t,tm);	
	
	return(0);
}

int
RTCFUNC(set,rtc8xx)(struct tm *tm, int cent_reg) {
	time_t		t;
	
	
	//
	//	make sure RTC is on
	//

	if(!(*(chip_base.p+1) & 0x01)) 
		// turn on RTC in RTCSC
	   	*(chip_base.p+1) |= 0x01;
	
#ifdef DIAG
	fprintf(stderr,"RTCSC		%x\n",*(short int *)chip_base.p);
#endif

	t = mktime(tm);

	//
	//	mktime assumes local time.  We will subtract
	//	timezone
	//

	t -= timezone;

	*(long *)(chip_base.p+4)=t;

#ifdef DIAG
	printf(stderr,"RTC		%x\n",*(int *)(chip_base.p+4));
#endif

	return(0);
}
