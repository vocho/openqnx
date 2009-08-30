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
#include <sys/timeb.h>

int ftime(struct timeb *timeptr) {
	struct timespec		ts;
	time_t				timer;
	struct tm			*tm_ptr, tm_struct;

	if(clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		return -1;
	}

	timer = (time_t)ts.tv_sec;
	if(ts.tv_nsec >=  500000000L) {
		timer++;
	}

	tm_ptr = localtime_r(&timer, &tm_struct);

	timeptr->dstflag	= tm_ptr->tm_isdst;
	timeptr->time		= (time_t) ts.tv_sec;
	timeptr->millitm	= ts.tv_nsec / 1000000;
	timeptr->timezone	= -tm_ptr->tm_gmtoff / 60;
 
	return 0;
}

__SRCVERSION("ftime.c $Rev: 153052 $");
