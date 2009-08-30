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
#include <inttypes.h>


unsigned
sleep(unsigned seconds) {
	uint64_t		it, ot;

	it = seconds * (uint64_t)1000000000;

	(void)TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_NANOSLEEP, NULL, &it, &ot);

	return ot / (uint64_t)1000000000;
}

__SRCVERSION("sleep.c $Rev: 153052 $");
