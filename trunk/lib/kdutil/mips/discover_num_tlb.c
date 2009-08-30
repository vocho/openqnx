/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <sys/syspage.h>
#include <mips/priv.h>
#include "kdintl.h"

#define HAVE_WIRED() ((__cpu_flags & MIPS_CPU_FLAG_NO_WIRED) == 0)

unsigned 
discover_num_tlb(unsigned shift) {
	unsigned	high;
	unsigned	v;
	unsigned	i;
	unsigned	orig_wired;
	unsigned	tries;

	if(HAVE_WIRED()) {
		orig_wired = CP0REG_GET(6);
	} else {
		orig_wired = 8;
	}
	i = 0;
	high = 0;
	tries = 128;
	do {
		v = CP0REG_GET(1) >> shift; // Get RANDOM value
		if(v > high) {
			high = v;
			if(HAVE_WIRED()) {
				CP0REG_SET(6, high);
			} else {
				tries = high * 4;
			}
			i = 0;
		}
	} while(++i < tries);
	if(HAVE_WIRED()) { 
		CP0REG_SET(6, orig_wired);
	}
	// If going to return odd number, make it even. R4700's come out
	// of the above indicating 47 entries for some reason.
	if((high & 1) == 0) ++high; 
	return high + 1;
}
