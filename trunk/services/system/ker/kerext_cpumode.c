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

#include "externs.h"


/*
 * Change the power management mode for the CPU.
 */

static void
cpumode(void *mode) {
	int			r = ENOSYS;
	uint64_t	tspec;

	timer_next(&tspec);

	lock_kernel();
	if(calloutptr->power != NULL) {

		// The top bit on tells the callout that it's a power level
		// setting, the low order bits are what level to set it to.

		r = calloutptr->power(_syspage_ptr, (int)mode | 0x80000000, &tspec);
	}
	SETKSTATUS(actives[KERNCPU], r);
}


int
SysCpumode(int mode) {
	return __Ring0(cpumode, (void *)mode);
}

__SRCVERSION("kerext_cpumode.c $Rev: 153052 $");
