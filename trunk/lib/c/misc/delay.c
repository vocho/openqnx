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
#include <time.h>
#include <sys/neutrino.h>


unsigned delay(unsigned msec) {
	struct timespec tim;

	tim.tv_sec = msec/1000;
	tim.tv_nsec = 1000000 * (msec%1000);
	(void)clock_nanosleep(CLOCK_REALTIME, 0, &tim, &tim);

	return(tim.tv_sec*1000 + tim.tv_nsec/1000000);
	}

__SRCVERSION("delay.c $Rev: 153052 $");
