/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */





#include "rtc.h"

#include <sys/timers.h>
#include <sys/vc.h>
#include <sys/kernel.h>
#include <util/prerror.h>

static	pid_t			procpid;

int
init_net(struct chip_loc *chip, char *argv[]) {
	nid_t 			node;

	if(argv[0] == NULL) {
		node = 1;
	} else {
		node = strtol(argv[0], NULL, 0);
	}

	/* establish vc to remote proc */
	if ((procpid = qnx_vc_attach(node,PROC_PID,0,_VC_AT_SHARE))==-1) {
		prerror("Can't create VC to node %d",node);
		return(-1);
	}
	return 0;
}

int
get_net(struct tm *tm, int cent_reg) {
	struct timespec	times;

	if (qnx_getclock(procpid,TIMEOFDAY,&times)==-1) {
		perror("qnx_getclock");
		return(-1);	/* vc will get torn down by term code */
	}
	*tm = *gmtime(&times.tv_sec);
	return(0);
}

int
set_net(struct tm *tm, int cent_reg) {
	struct timespec	times;

	times.tv_sec = mktime(tm);
	times.tv_nsec = 0L;
	if (qnx_setclock(procpid,TIMEOFDAY,&times)==-1) {
		perror("qnx_setclock");
		return(-1);	/* vc will get torn down by term code */
	}
	return(0);
}
