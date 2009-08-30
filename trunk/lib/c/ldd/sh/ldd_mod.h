/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */



/* this function is required to get around the SH gcc 3.2.3 compiler */

static __inline__ unsigned long
static_mod(unsigned long num, unsigned long den)
{
	unsigned long bit = 1;
	unsigned long res = 0;

	while (den < num && bit && !(den & (1L<<31))) {
		den <<=1;
		bit <<=1;
	}
	while (bit) {
		if (num >= den) {
			num -= den;
			res |= bit;
		}
		bit >>=1;
		den >>=1;
	}
	return num;
}

/* __SRCVERSION("ldd_mod.h $Rev: 153052 $"); */
