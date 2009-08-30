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
#include <sys/neutrino.h>
#include <errno.h>

#define MAX_TIME (500 * 1000 * 1000)   /* 500 MS */

int
nanospin_ns(unsigned long nsec) {
	unsigned long	count;

    /*
     * Return Error if wasting time
     */
    if(nsec >= MAX_TIME) {
		return E2BIG;
    }
	count = nanospin_ns_to_count(nsec);
	if(count == -1UL) {
		return errno;
	}
	nanospin_count(count);

    return EOK;
}

__SRCVERSION("nanospin_ns.c $Rev: 153052 $");
