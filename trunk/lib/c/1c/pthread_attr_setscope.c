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
#include <sys/neutrino.h>

int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope) {
	if(contentionscope == PTHREAD_SCOPE_SYSTEM) {
		attr->__flags = (attr->__flags & ~PTHREAD_CONTENTIONSCOPE_MASK) | contentionscope;
	    return EOK;
	}
	if(contentionscope == PTHREAD_SCOPE_PROCESS) {
		return ENOTSUP;
	}
	return EINVAL;
}

__SRCVERSION("pthread_attr_setscope.c $Rev: 153052 $");
