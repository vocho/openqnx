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
#include <sys/neutrino.h>

int nanosleep_rel(clockid_t clock_id, const struct timespec *rqtp, struct timespec *rmtp) {
	// Passing a NULL for event is the same as notify = SIGEV_UNBLOCK
	return(timer_timeout(clock_id, (1 << STATE_NANOSLEEP), 0, rqtp, rmtp));
}

__SRCVERSION("nanosleep_rel.c $Rev: 153052 $");
