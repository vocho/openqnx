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
#include <stdlib.h>
#include <string.h> /* for memset */
#include <pthread.h> 
#include "pthread_key.h"
#include "cpucfg.h"

static struct keyinfo {
	unsigned					numkeys;
	void						**keydata;
}							*keyarray;
static int					keysize;
static pthread_mutex_t		keymutex = PTHREAD_MUTEX_INITIALIZER;

int
pthread_setspecific(pthread_key_t key, const void *value) {
	void									**temp;
	int										status;
	volatile struct _thread_local_storage	*tls = LIBC_TLS();
	unsigned								tid_index = tls->__tid - 1;

	/* Is the key invalid? */
	if(key < 0 || key >= PTHREAD_KEYS_MAX || !_key_destructor || _key_destructor[key] == _KEY_NONE) {
		return EINVAL;
	}

	/* No other thread can access this threads data so just stuff it if it was used before */
	if(key < tls->__numkeys) {
		tls->__keydata[key] = (void *)value;
		return EOK;
	}

	/* We will be accessing a global array, so protect it!! */
	if((status = pthread_mutex_lock(&keymutex)) != EOK) {
		return status;
	}

	if(!tls->__keydata) {
		/* this is this the first pthread_setspecific for this thread */
		if(tid_index >= keysize) {
			/* this is the largest tid in the process to use pthread_setspecific */
			struct keyinfo						*tmp;

			/* grow the array to remember the memory globaly */
			if(!(tmp = realloc(keyarray, (tls->__tid) * sizeof *tmp))) {
				pthread_mutex_unlock(&keymutex);
				return ENOMEM;
			}

			/* initialize the new entries */
			keyarray = tmp;
			memset(&(keyarray[keysize]), 0, sizeof *tmp * (tls->__tid - keysize));
			keysize = tls->__tid;
		}

		/* The tid was used before, re-use the thread data, but zero it!! */
		tls->__keydata = keyarray[tid_index].keydata;
		tls->__numkeys = keyarray[tid_index].numkeys;
		memset(tls->__keydata, 0x00, sizeof *tls->__keydata * tls->__numkeys);

		/* See if there is a spot now? */
		if(key < tls->__numkeys) {
			pthread_mutex_unlock(&keymutex);
			tls->__keydata[key] = (void *)value;
			return EOK;
		}
	}

	/* Must grow the thread specific key array */
	if(!(temp = realloc(tls->__keydata, (key + 1) * sizeof *temp))) {
		pthread_mutex_unlock(&keymutex);
		return ENOMEM;
	}

	/* Remember the information in both the global array and the local tls */
	keyarray[tid_index].keydata = tls->__keydata = temp;
	memset(&(tls->__keydata[tls->__numkeys]), 0, sizeof *temp * ((key + 1) - tls->__numkeys));
	keyarray[tid_index].numkeys = tls->__numkeys = key + 1;

	/* We are done */
	pthread_mutex_unlock(&keymutex);
	tls->__keydata[key] = (void *)value;
	return EOK;
}

void
_key_delete(pthread_key_t key)
{
	int		i;

	/*
	 * Reset key value if key had been previously used
	 */
	for (i = 0; i < keysize; i++) {
		if (keyarray[i].keydata && key < keyarray[i].numkeys) {
			keyarray[i].keydata[key] = NULL;
		}
	}
}

__SRCVERSION("pthread_setspecific.c $Rev: 153052 $");
