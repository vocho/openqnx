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





#include <sched.h>
#include <sys/neutrino.h>

int
sched_get_priority_adjust(int prio, int alg, int adjust) {
	struct _sched_info	info;
	struct sched_param	param;
	int					status;
	int					curr_alg;

	curr_alg = SchedGet_r(0, 0, &param);
	if(curr_alg < 0) return curr_alg;
	if(alg == SCHED_NOCHANGE) alg = curr_alg;
	if(prio < 0) prio = param.sched_curpriority;

	status = SchedInfo_r(0, alg, &info);
	if(status < 0) return status;

	prio += adjust;
	if(prio > info.priority_max) prio = info.priority_max;
	if(prio < info.priority_min) prio = info.priority_min;

	return prio;
}

__SRCVERSION("sched_get_priority_adjust.c $Rev: 153052 $");
