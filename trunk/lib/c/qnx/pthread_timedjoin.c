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
#include <pthread.h> 
#include <sys/neutrino.h>
#include <inttypes.h>

int pthread_timedjoin(pthread_t thread, void **value_ptr, const struct timespec *abstime) {
	int			rc;
	uint64_t	t = timespec2nsec(abstime);

	while ((rc = TimerTimeout_r(CLOCK_REALTIME, TIMER_ABSTIME | _NTO_TIMEOUT_JOIN, 0, &t, 0)) == EOK) {
		rc = ThreadJoin_r(thread, value_ptr);
		if (rc != EINTR) {
			break;
		}
	}

	return rc;
}

__SRCVERSION("pthread_timedjoin.c $Rev: 153052 $");
