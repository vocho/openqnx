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
#include <sys/resource.h>

int  setpriority(int which, id_t who, int priority) {
#if 0
	int				max;

	if((max = sysconf(_SC_NZERO)) == -1) {
		return -1;
	}
	max = max * 2 - 1;	// keep between 0 and NZERO*2-1 (UNIX98)

	if(priority < 0) {
		priority = 0;
	} else if(priority > max) {
		priority = max;
	}
#else
	// We only support realtime schedulers so ignore the priority (UNIX98)
	return 0;
#endif
}

__SRCVERSION("setpriority.c $Rev: 153052 $");
