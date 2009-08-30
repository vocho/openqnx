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
 * Make the system reboot itself.
 */

static void
reboot(void *abnormal) {
	lock_kernel();
	cpu_reboot();
	calloutptr->reboot(_syspage_ptr, (int)abnormal);
}


void
RebootSystem(int abnormal) {
	if(!am_inkernel()) {
		__Ring0(reboot, (void *)abnormal);
	}
	reboot((void *)abnormal);
	for(;;) {
	}
}

__SRCVERSION("kerext_reboot.c $Rev: 153052 $");
