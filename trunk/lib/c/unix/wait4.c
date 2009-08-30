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




#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

static void
cvt_time(struct timeval *tvp, clock_t clk) {
	ldiv_t	res;

	res = ldiv(clk, CLOCKS_PER_SEC);
	tvp->tv_sec  = res.quot;
	tvp->tv_usec = (res.rem * 1000000) / CLOCKS_PER_SEC;
}

pid_t
wait4(pid_t pid, int *stat_loc, int options, struct rusage *resource_usage) {
	siginfo_t				info;
    idtype_t				idtype;

	if(pid == 0) {
		idtype = P_ALL;
	} else if(pid <= 0) {
		pid = -pid;
		idtype = P_PGID;
	} else {
		idtype = P_PID;
	}
	if(waitid(idtype, pid, &info, options | WEXITED | WTRAPPED) == -1) {
		return -1;
	}
	if(info.si_signo != SIGCHLD) {
		return 0;
	}
	if(stat_loc) {
		switch(info.si_code) {
		case CLD_EXITED:
			*stat_loc = (info.si_status & 0xff) << 8;
			break;
		case CLD_KILLED:
			*stat_loc = info.si_status & WSIGMASK;
			break;
		case CLD_DUMPED:
			*stat_loc = (info.si_status & WSIGMASK) | WCOREFLG;
			break;
		case CLD_TRAPPED:
		case CLD_STOPPED:
			*stat_loc = ((info.si_status & WSIGMASK) << 8) | WSTOPFLG;
			break;
		case CLD_CONTINUED:
			*stat_loc = WCONTFLG;
			break;
		default:
			errno = EINVAL;
			return -1;
		}
	}
	if(resource_usage != NULL) {
		memset(resource_usage, 0, sizeof(*resource_usage));
		cvt_time(&resource_usage->ru_utime, info.si_utime);
		cvt_time(&resource_usage->ru_stime, info.si_stime);

		//Fill in other info as it becomes available

	}
	return info.si_pid;
}

__SRCVERSION("wait4.c $Rev: 153052 $");
