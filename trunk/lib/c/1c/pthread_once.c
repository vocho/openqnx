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

static void pthread_once_cancel(void *data) {
	pthread_mutex_t		*mutex = data;

	pthread_mutex_unlock(mutex);
}

int __pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
	int					ret;

	if((ret = pthread_mutex_lock(&once_control->__mutex)) != EOK) {
		return (once_control->__once != 0) ? EOK : ret;
	}

	if( once_control->__once == 0 ) {
		pthread_cleanup_push(pthread_once_cancel, &once_control->__mutex);
		init_routine();
		pthread_cleanup_pop(0);
		once_control->__once = 1;
	}

	if(pthread_mutex_unlock(&once_control->__mutex) == EOK) {
		pthread_mutex_destroy(&once_control->__mutex);
	}
	return EOK;
}

#undef pthread_once
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
	if(once_control->__once == 0) {
		return __pthread_once(once_control, init_routine);
	}
	return EOK;
}

__SRCVERSION("pthread_once.c $Rev: 153052 $");
