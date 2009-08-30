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

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
	unsigned	flags;

	flags = (attr->__flags & ~(PTHREAD_MUTEX_TYPE |
				PTHREAD_RECURSIVE_MASK | PTHREAD_ERRORCHECK_MASK)) |
				_NTO_ATTR_EXTRA_FLAG;

	switch(type) {
	case PTHREAD_MUTEX_DEFAULT:
		attr->__flags = flags |
			(PTHREAD_RECURSIVE_DISABLE | PTHREAD_ERRORCHECK_ENABLE);
		break;
	case PTHREAD_MUTEX_ERRORCHECK:
		attr->__flags = flags |
			(PTHREAD_MUTEX_TYPE | PTHREAD_RECURSIVE_DISABLE | PTHREAD_ERRORCHECK_ENABLE);
		break;
	case PTHREAD_MUTEX_RECURSIVE:
		attr->__flags = flags |
			(PTHREAD_MUTEX_TYPE | PTHREAD_RECURSIVE_ENABLE);
		break;
	case PTHREAD_MUTEX_NORMAL:
		attr->__flags = flags |
			(PTHREAD_MUTEX_TYPE | PTHREAD_RECURSIVE_DISABLE | PTHREAD_ERRORCHECK_DISABLE);
		break;
	default:
		return EINVAL;
	}
	return EOK;
}

__SRCVERSION("pthread_mutexattr_settype.c $Rev: 153052 $");
