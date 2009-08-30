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




#include <sys/neutrino.h>
#include <sys/time.h>

int
settimeofday(const struct timeval *tp, const struct timezone *tzp) {
	struct timespec ts;

	ts.tv_sec = tp->tv_sec;
	ts.tv_nsec = tp->tv_usec*1000;
	return clock_settime(CLOCK_REALTIME, &ts);
}

__SRCVERSION("settimeofday.c $Rev: 153052 $");
