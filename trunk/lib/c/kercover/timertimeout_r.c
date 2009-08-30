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
#include <time.h>
#include <sys/neutrino.h>

int timer_timeout_r(clockid_t id, int flags, const struct sigevent *notify,
		const struct timespec *ntime, struct timespec *otime) {
	int				ret;
	uint64_t		o, n;

	if(ntime) {
		n = timespec2nsec(ntime);
	}

	ret = TimerTimeout_r(id, flags, notify, ntime ? &n : 0, otime ? &o : 0);

	if(otime) {
		nsec2timespec(otime, o);
	}

	return ret;
}


__SRCVERSION("timertimeout_r.c $Rev: 153052 $");
