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
#include <string.h> /* for memset */
#include <pthread.h> 

/*
 * Remember this functions is used by pthread_rwlockattr_init()
 * and barrier_attr_init() in the initial release of neutrino.
 */

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
#ifndef IGNORE_OLD_SYNCATTR
	// This is needed for backwards compatibility with 2.00 compiled binaries
	attr->__prioceiling = 0;
#else
	memset(attr, 0, sizeof *attr);
#endif
	attr->__flags = _NTO_ATTR_MUTEX;
	attr->__protocol = PTHREAD_PRIO_INHERIT;
	return EOK;
}

__SRCVERSION("pthread_mutexattr_init.c $Rev: 153052 $");
