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
#include <sched.h>
#include <sys/neutrino.h>

int (sched_rr_get_interval)(pid_t pid, struct timespec *t) {
	struct _sched_info		info;

	if(SchedInfo(pid, SCHED_RR, &info) == -1) {
		return -1;
	}
	if(t) {
		nsec2timespec(t, info.interval);
	}
	return 0;
}

__SRCVERSION("sched_rr_get_interval.c $Rev: 153052 $");
