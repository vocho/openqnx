/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems. All Rights Reserved.
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
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/neutrino.h>

useconds_t ualarm(useconds_t usec, useconds_t interval) {
	struct _itimer		i, o;
	useconds_t			ret;

	i.nsec = usec * (uint64_t) 1000;
	i.interval_nsec = interval * (uint64_t) 1000;

	(void) TimerAlarm(CLOCK_REALTIME, & i, & o);

	/* this call cannot return 0, unless the timer has fired */
	ret = o.nsec / 1000;
	return (o.nsec && ret == 0) ? 1 : ret;
}

__SRCVERSION("ualarm.c $Rev: 209348 $");
