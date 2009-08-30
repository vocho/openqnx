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

#include "mallocint.h"
#include <time.h>

#define BILLION		1000000000L
#define MILLION		1000000L
#define THOUSAND	1000L


static struct timespec start_time;

void _reset_timestamp() {
	if (clock_gettime(CLOCK_REALTIME, &start_time) == -1) {
		return;
	}
}

uint64_t _get_timestamp() {
	struct timespec stop;
	uint64_t timediff;

	if (clock_gettime( CLOCK_REALTIME, &stop) == -1) {
		return -1;
	}


	timediff = (stop.tv_sec - start_time.tv_sec) * MILLION +
		(stop.tv_nsec - start_time.tv_nsec) / THOUSAND;
	return timediff;
}
