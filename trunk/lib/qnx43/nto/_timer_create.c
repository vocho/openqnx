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
#include <time.h>
#include "proto.h"

int _timer_create(clockid_t clock_id, struct sigevent *evp) {
	timer_t						timerid;
	struct sigevent				event;

	if(evp->sigev_signo >= 0) {
		event = *evp;
		event.sigev_notify = SIGEV_SIGNAL;
	} else {
		event.sigev_notify = SIGEV_PULSE;
		event.sigev_coid = pid2coid(event.sigev_value.sival_int = -evp->sigev_signo);
		event.sigev_code = SI_USER;
		event.sigev_priority = -1;
	}
	return timer_create(clock_id, &event, &timerid) == -1 ? -1 : timerid;
}
