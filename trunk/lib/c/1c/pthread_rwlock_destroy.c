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
#include "cpucfg.h"

int pthread_rwlock_destroy(pthread_rwlock_t *l)
{
	int status, id = LIBC_TLS()->__owner;

	if ((status = pthread_mutex_lock(&l->__lock)) == EOK) {
		if ((l->__owner == id) || (l->__active == 0)) {
			l->__heavy = l->__active = l->__owner = -2U;
			pthread_cond_broadcast(&l->__rcond);
			pthread_cond_destroy(&l->__rcond);
			pthread_cond_broadcast(&l->__wcond);
			pthread_cond_destroy(&l->__wcond);
			return pthread_mutex_destroy(&l->__lock);
		} else {
			status = EBUSY;
		}

		pthread_mutex_unlock(&l->__lock);
	}

	return status;
}


__SRCVERSION("pthread_rwlock_destroy.c $Rev: 153052 $");
