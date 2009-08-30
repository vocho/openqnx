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
#include <pthread.h> 
#include <unistd.h>
#include <sys/neutrino.h>

int pthread_attr_init(pthread_attr_t *attr) {
	static long		pagesize;
	int				ret;

	*attr = pthread_attr_default;
	if((ret = SchedGet_r(0, 0, (struct sched_param *)&attr->__param)) < 0) {
		return -ret;
	}
	attr->__policy = ret;

	if(pagesize == 0) {
		ret = errno;
		if((pagesize = sysconf(_SC_PAGESIZE)) == -1) {
			errno = ret;
		}
	}

	attr->__guardsize = pagesize <= 0 ? 0 : pagesize;

	return EOK;
}

__SRCVERSION("pthread_attr_init.c $Rev: 153052 $");
