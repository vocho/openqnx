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




#include <inttypes.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/times.h>
#include <sys/resource.h>

clock_t
times(struct tms *tms) {
	clock_t				tck = sysconf(_SC_CLK_TCK);
	struct timespec		ts;
	struct rusage		ru;

	if(getrusage(RUSAGE_SELF, &ru) == -1) {
		return (clock_t)-1;
	}
	tms->tms_utime = ru.ru_utime.tv_sec * tck +
		ru.ru_utime.tv_usec * tck / 1000000;
	tms->tms_stime = ru.ru_stime.tv_sec * tck +
		ru.ru_stime.tv_usec * tck / 1000000;

	if(getrusage(RUSAGE_CHILDREN, &ru) == -1) {
		return (clock_t)-1;
	}
	tms->tms_cutime = ru.ru_utime.tv_sec * tck +
		ru.ru_utime.tv_usec * tck / 1000000;
	tms->tms_cstime = ru.ru_stime.tv_sec * tck +
		ru.ru_stime.tv_usec * tck / 1000000;

	if(clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
		return (clock_t)-1;
	}
	return ts.tv_sec * tck +
		(unsigned)ts.tv_nsec / ((uint64_t)1000000000 / tck);
}

__SRCVERSION("times.c $Rev: 153052 $");
