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




#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h> 
#include "pthread_key.h"

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
	int					ret;
	pthread_key_t		k;

	if((ret = pthread_mutex_lock(&_key_mutex)) != EOK) {
		return ret;
	}

	// allocate destructor table
	if(!_key_destructor) {
		if(!(_key_destructor = malloc(sizeof *_key_destructor * PTHREAD_KEYS_MAX))) {
			pthread_mutex_unlock(&_key_mutex);
			return ENOMEM;
		}
		memset(_key_destructor, (int)_KEY_NONE, sizeof *_key_destructor * PTHREAD_KEYS_MAX);
	}

	// is destructor valid?
	if(destructor == _KEY_NONE ) {
		pthread_mutex_unlock(&_key_mutex);
		return EINVAL;
	}

	// are there enough keys?
	if(_key_count >= PTHREAD_KEYS_MAX) {
		pthread_mutex_unlock(&_key_mutex);
		return EAGAIN;
	}

	// find first free key
	for(k = 0; k < PTHREAD_KEYS_MAX; k++) {
		if(_key_destructor[k] == _KEY_NONE) {
			break;
		}
	}

	// this should never happen!
	if(k >= PTHREAD_KEYS_MAX) {
		pthread_mutex_unlock(&_key_mutex);
		return EAGAIN;
	}

	_key_destructor[k] = destructor;
	*key = k;
	_key_count++;

	pthread_mutex_unlock(&_key_mutex);
	return EOK;
}

__SRCVERSION("pthread_key_create.c $Rev: 153052 $");
