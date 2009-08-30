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




#include <limits.h>
#include <errno.h>
#include <pthread.h> 
#include "pthread_key.h"

int pthread_key_delete(pthread_key_t key) {
	int						ret;

	if((ret = pthread_mutex_lock(&_key_mutex)) != EOK) {
		return ret;
	}

	if(key < 0 || key >= PTHREAD_KEYS_MAX || !_key_destructor || _key_destructor[key] == _KEY_NONE) {
		pthread_mutex_unlock(&_key_mutex);
		return EINVAL;
	}

	// destructor functions are called in pthread_exit
	_key_destructor[key] = _KEY_NONE;
	_key_count--;
	_key_delete(key);
	pthread_mutex_unlock(&_key_mutex);
	return EOK;
}


__SRCVERSION("pthread_key_delete.c $Rev: 153052 $");
