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
#include <inttypes.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

uint64_t
_syspage_time(clockid_t clock_id)
{
	uint64_t nsec, adjust;
	static struct qtime_entry *qtp;

	if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC) {
		if (ClockTime(clock_id, 0, &nsec) == -1)
			return 0;

		return nsec;
	}

	if (qtp == NULL)
		qtp = SYSPAGE_ENTRY(qtime);

	/*
	 * Loop until we get two time values the same in case an interrupt
	 * comes in while we're reading the value.
	 */
	if(qtp->flags & QTIME_FLAG_CHECK_STABLE) {
		do {
			nsec = qtp->nsec;
			adjust = qtp->nsec_tod_adjust;
		} while(nsec != qtp->nsec_stable);
	} else {
		/*
		 * after a suitable period of time, we can probably remove
		 * this code and just assume that we're using a procnto
		 * that sets the nsec_stable field 2008/08/18
		 */
		do {
			nsec = qtp->nsec;
			adjust = qtp->nsec_tod_adjust;
		} while(nsec != qtp->nsec || adjust != qtp->nsec_tod_adjust);
	}

	if (qtp->nsec_inc == 0 || nsec == (-(uint64_t)1)) {
		/*
		 * If nsec field is -1, power managment has kicked in.
		 * If nsec_inc field is 0, there is no ticker
		 */
		if (ClockTime(clock_id, 0, &nsec) == -1)
			return 0;
	}
	else if (clock_id == CLOCK_REALTIME) {
		nsec += adjust;
	}

	return nsec;
}

__SRCVERSION("_syspage_time.c $Rev: 175178 $");
