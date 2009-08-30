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

#define	AT_CNTL		0x70		/* AT CMOS, clock control register.	*/
#define CLK_HOLD	0x0080		/* Value used to set bit 7 of Reg B */
#define H2412		0x0002		/* Used to force 24 hour format		*/
#define BIN			0x0004		/* data in binary, not BCD			*/
#define REG_A		0x0A		/* MC146818 Status Register A		*/
#define	REG_B		0x0B		/* MC146818 Status Register B		*/
#define CMOS_CNT	0xffff		/* Number of retries when testing Reg. A */

static	unsigned
cmos(unsigned i) {
	unsigned temp;

	_disable();		
	chip_write8(0,i);
	temp = chip_read8(1);
	_enable();		
	return(temp);
}


static void
cmos_set(unsigned i, unsigned d) {
	_disable();
	chip_write8(0,i);
	chip_write8(1,d);
	_enable();	
}

int
RTCFUNC(init,mc146818)(struct chip_loc *chip, char *argv[]) {
	if(chip->phys == NIL_PADDR) {
#ifdef __QNXNTO__
		fprintf(stderr,"rtc: -b baseaddr must be specified for mc146818 clock type\n");
		return(-1);
#else
		chip->phys = AT_CNTL;
#endif
	}
#ifdef NEVER
	if(chip->century_reg == UNSET) 
			chip->century_reg = 50;
#endif
	if(chip->access_type == NONE) 
			chip->access_type = IOMAPPED;
	return(2);
}

int
RTCFUNC(get,mc146818)(struct tm *tm, int cent_reg) {
	unsigned	x = 0;
	unsigned	count;
	unsigned	clk_buf[2][7];
	int			z;

	x = cmos(REG_A);
	if ((x & 0x60) != 0x20) {
		chip_write8 (1, (x | 0x60));	//Check for ATI IXP200 RTC - these bits are reserved
		if ((cmos(REG_A) & 0x60) != 0) {
			chip_write8 (1, x);		//restore old value
			return(-1);
			}
		}

	memset(clk_buf, 0, sizeof(clk_buf));

	//Make sure the UIP turns on at some time.
	for(x = 0, z = 1 ; (z <= 2) && !(x & 0x0080) ; z++) {
		for(count=CMOS_CNT ; !(x & 0x0080) && count ; count--) {
			x = cmos(REG_A);
		}
	}

	for( ;; ) {
		//Make sure the UIP bit is off.
		for(x=0x080, count = CMOS_CNT ; (x & 0x0080) && count ; count--) {
			x = cmos(REG_A);
		}

		if(count == 0) {
			fprintf( stderr, "RTC: cannot read AT clock\n");
			return(-1);
		}
			
		clk_buf[0][0] = cmos(0);		/* seconds	*/
		clk_buf[0][1] = cmos(2);		/* minutes	*/
		clk_buf[0][2] = cmos(4);		/* hours	*/
		clk_buf[0][3] = cmos(7);		/* day		*/
		clk_buf[0][4] = cmos(8);		/* month	*/
		clk_buf[0][5] = cmos(9);		/* year		*/

		if (cent_reg >= 0) {
			clk_buf[0][6] = cmos(cent_reg);		/* century  */
		}
		x = cmos(REG_B);

		if(memcmp(clk_buf[0], clk_buf[1], sizeof(clk_buf[0])) == 0) break;
		memcpy(clk_buf[1], clk_buf[0], sizeof(clk_buf[0]));

	}
	if(!(x & BIN)) {
		clk_buf[0][0] = BCD2BIN(clk_buf[0][0]);
		clk_buf[0][1] = BCD2BIN(clk_buf[0][1]);
		clk_buf[0][2] = BCD2BIN(clk_buf[0][2] & 0x7f);
		clk_buf[0][3] = BCD2BIN(clk_buf[0][3]);
		clk_buf[0][4] = BCD2BIN(clk_buf[0][4]);
		clk_buf[0][5] = BCD2BIN(clk_buf[0][5]);
		clk_buf[0][6] = BCD2BIN(clk_buf[0][6]);
	}

	tm->tm_sec = clk_buf[0][0];
	tm->tm_min = clk_buf[0][1];

	tm->tm_hour = clk_buf[0][2] & 0x7f;
	if (!(x & H2412) && (clk_buf[1][2] & 0x80)) {	 /* 12 hour format, PM */
		tm->tm_hour += 12;
	}
	tm->tm_mday		= clk_buf[0][3];
	tm->tm_mon		= clk_buf[0][4] - 1;
	tm->tm_year		= clk_buf[0][5];

#ifdef VERBOSE_SUPPORTED
	/* print diagnostics on read if double -v (-vv) specified */
	if (verbose>1) {
		fprintf(stdout,"cmos(0) seconds=%d\n",tm->tm_sec);
		fprintf(stdout,"cmos(2) minutes=%d\n",tm->tm_min);
		fprintf(stdout,"cmos(4) hour   =%d\n",tm->tm_hour);
		fprintf(stdout,"cmos(7) day    =%d\n",tm->tm_mday);
		fprintf(stdout,"cmos(8) month  =%d\n",tm->tm_mon + 1);
		fprintf(stdout,"cmos(9) year   =%d\n",tm->tm_year);
		fprintf(stdout,"(unused: cmos(50) century = %d)\n",clk_buf[0][6]);
	}
#endif

	if (cent_reg >= 0) {
		if ( clk_buf[0][6] ) tm->tm_year += 100;
	} else if (tm->tm_year < 70) {
		tm->tm_year += 100;
	}

	return(0);
}

int
RTCFUNC(set,mc146818)(struct tm *tm, int cent_reg) {
	unsigned	seconds;
	unsigned	minutes;
	unsigned	hours;
	unsigned	day;
	unsigned	month;
	unsigned	year;
	unsigned	cent;
	unsigned	x;

	x = cmos(REG_A);
	x = x & 0xf | 0x20;	/* Set internal ocillator running */
	cmos_set(REG_A, x);
	
	x = cmos(REG_B);
	x |= CLK_HOLD | H2412;	/* Set bit 7 & 1 of Status Register B to a '1',   */
	cmos_set(REG_B, x);	/* Hold clock updates, clear UIP bit in REG A */ 

	seconds	= tm->tm_sec;
	minutes	= tm->tm_min;
	hours	= tm->tm_hour;
	day 	= tm->tm_mday;
	month	= tm->tm_mon + 1;
	year	= tm->tm_year % 100;
	cent	= (tm->tm_year / 100) + 19;
#ifdef DIAG
	fprintf(stderr,"rtc set: cent=%d; year=%d (after adjustment)\n",cent,year);
#endif

	if(!(x & BIN)) {
		seconds	= BIN2BCD(seconds);
		minutes	= BIN2BCD(minutes);
		hours	= BIN2BCD(hours);
		day 	= BIN2BCD(day);
		month	= BIN2BCD(month);
		year	= BIN2BCD(year);
		cent	= BIN2BCD(cent);
	}

	cmos_set(7, day);
	cmos_set(8, month);
	cmos_set(9, year);

	if (cent_reg >= 0) cmos_set(cent_reg,cent);
	cmos_set(4, hours);
	cmos_set(2, minutes);
	cmos_set(0, seconds);
	x &= ~CLK_HOLD;
	cmos_set(REG_B, x);	/* Restart the clock.	*/

	return(0);
}
