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




#include <stdio.h>		// To set _multi_threaded
#include <errno.h>
#include <pthread.h> 
#include <sys/neutrino.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
	int					tid;

	_Multi_threaded = 1;
	while((tid = ThreadCreate_r(0, start_routine, arg, attr ? attr : &pthread_attr_default)) < 0) {
		if (tid != -EINTR) {
			return -tid;
		}
	}
	if(thread) {
		*thread = (pthread_t) tid;
	}
	return EOK;
}

__SRCVERSION("pthread_create.c $Rev: 153052 $");
