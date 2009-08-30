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




#include <errno.h>
#include <sched.h>
#include <sys/neutrino.h>
#include <sys/resource.h>

int nice(int incr) {
	int				value;
	int				err = errno;

	errno = 0;
	if((value = getpriority(PRIO_PROCESS, 0)) != -1 || errno == 0) {
		errno = err;
		// setpriority will make sure the value stays between 0 and NZERO*2-1
		return setpriority(PRIO_PROCESS, 0, value + incr);
	}
	return -1;
}

__SRCVERSION("nice.c $Rev: 153052 $");
