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




#include <time.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/neutrino.h>

int
clock_settime(clockid_t clock_id, const struct timespec *tp) {
	uint64_t		t;

	//Try and catch people who stick the result of mktime into the
	//struct timespec without checking the return value to see if it was
	//representable.
	//plus POSIX 14.2.1.4
	if(tp->tv_sec == (time_t)-1 || !TIMESPEC_VALID(tp)) {
		errno = EINVAL;
		return -1;
	}

	t = timespec2nsec(tp);

	return(ClockTime(clock_id, &t, 0));
}

__SRCVERSION("clock_settime.c $Rev: 166190 $");
