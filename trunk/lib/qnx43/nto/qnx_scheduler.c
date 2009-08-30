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
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/neutrino.h>

int qnx_scheduler(pid_t proc, pid_t pid, int alg, int prio, int ret) {
	struct sched_param param;
	int policy;

	if(proc  &&  proc != 1) {
		errno = EHOSTUNREACH;
		return(-1);
		}

	if((policy = SchedGet(pid, 1, &param)) == -1)
		return(-1);

	ret = ret ? policy : param.sched_priority;

	if(alg != -1  ||  prio != -1) {
		alg = (alg == -1) ? SCHED_NOCHANGE : alg;
		if(prio != -1) param.sched_priority = prio;
		if(SchedSet(pid, 1, alg, &param) == -1)
			return(-1);
		}

	return(ret);
	}
