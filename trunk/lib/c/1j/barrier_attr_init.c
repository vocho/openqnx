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




#include <pthread.h>
#include <errno.h>

int pthread_barrierattr_init(pthread_barrierattr_t *attr) {
#ifndef IGNORE_OLD_SYNCATTR
	// This is needed for backwards compatibility with 2.00 compiled binaries
	attr->__protocol = 0;
	attr->__prioceiling = 0;
#else
	memset(attr, 0, sizeof *attr);
#endif
	attr->__flags = _NTO_ATTR_BARRIER;
	return EOK;
}

/* Only for binary compatibility with 2.0 programs */
#include <sync.h>

int (barrier_attr_init)(barrier_attr_t *attr) {
	return pthread_barrierattr_init((pthread_barrierattr_t *)attr);
}

__SRCVERSION("barrier_attr_init.c $Rev: 153052 $");
