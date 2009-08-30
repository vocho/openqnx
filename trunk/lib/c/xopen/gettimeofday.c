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




#include "syspage_time.h"
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

int
gettimeofday(struct timeval *tp, void *zp)
{
	uint64_t nsec;

	if ((nsec = _syspage_time(CLOCK_REALTIME)) == 0)
		return -1;

	tp->tv_sec  = nsec / (uint64_t)1000000000;
	tp->tv_usec = (nsec % (uint64_t)1000000000) / 1000;

	zp = zp;

	return 0;
}

__SRCVERSION("gettimeofday.c $Rev: 153052 $");
