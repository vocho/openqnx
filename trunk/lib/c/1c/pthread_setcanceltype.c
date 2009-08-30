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
#include <atomic.h>
#include "cpucfg.h"

// This is an async-cancel safe function

int
pthread_setcanceltype(int type, int *oldtype) {
	int										oldflags;
	volatile struct _thread_local_storage	*tls = LIBC_TLS();

	switch( type ) 	{
	case PTHREAD_CANCEL_DEFERRED:
		oldflags = atomic_clr_value(&tls->__flags, PTHREAD_CANCEL_ASYNCHRONOUS);
		break;

	case PTHREAD_CANCEL_ASYNCHRONOUS:
		oldflags = atomic_set_value(&tls->__flags, PTHREAD_CANCEL_ASYNCHRONOUS);
		// If we were in a DEFERRED state before, and if cancelation is enabled,
		// check if a cancel request is pending
		if((oldflags & (PTHREAD_CSTATE_MASK | PTHREAD_CTYPE_MASK)) == (PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_DEFERRED)) {
			if((tls->__flags & PTHREAD_CANCEL_PENDING) == PTHREAD_CANCEL_PENDING) {
				pthread_exit(PTHREAD_CANCELED);
			}
		}
		break;

	default:
		return EINVAL;
	}

	if(oldtype) {
		*oldtype = oldflags & PTHREAD_CTYPE_MASK;
	}

	return EOK;
}

__SRCVERSION("pthread_setcanceltype.c $Rev: 204471 $");
