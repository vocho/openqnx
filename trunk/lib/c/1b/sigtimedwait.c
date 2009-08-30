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




#include <signal.h>
#include <sys/neutrino.h>
#include <inttypes.h>
#include <errno.h>

int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout) {
	int rc;

	if(timeout) {
		// Passing a NULL for event is the same as notify = SIGEV_UNBLOCK
		uint64_t	t = timespec2nsec(timeout);

		//POSIX 3.3.8.4 error checking
		if(!TIMESPEC_VALID(timeout)) {
			errno = EINVAL;
			return -1;
		}

		// Passing a NULL for event is the same as notify = SIGEV_UNBLOCK
		if(TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_SIGWAITINFO, 0, &t, 0) == -1) {
			return -1;
		}
	}
	
	// POSIX P1003.1-1996 Section 3.3.8.2 line 1995 says we have to do this
	if (((rc = SignalWaitinfo(set, info)) == -1) && (errno == ETIMEDOUT)) {
		errno = EAGAIN;
	}

	return rc;
}

__SRCVERSION("sigtimedwait.c $Rev: 166190 $");
