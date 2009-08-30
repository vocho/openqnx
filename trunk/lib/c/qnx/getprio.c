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

/*
 * This code is only here for porting old style single threaded code to
 * Neutrino.  Since an single threaded process will only have single
 * thread with an id of 1, the following implementation of getprio()
 * should allow qnx4 to compile and work correctly with Neutrino.
 * This routine should not be used with multi-thread programs.
 */

int getprio(pid_t pid) {
	struct sched_param params;
	int	status;

	status = SchedGet(pid, (pid == 0) ? 0 : 1, &params);

	return (status != -1) ? (params.sched_priority) : -1;
	}

__SRCVERSION("getprio.c $Rev: 153052 $");
