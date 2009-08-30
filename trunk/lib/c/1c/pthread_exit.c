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





#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <atomic.h>
#include <sys/neutrino.h>
#include "pthread_key.h"
#include "cpucfg.h"
#include "xmtx.h"

void
pthread_exit(void *value_ptr) {
	struct __cleanup_handler				*handler;
	int										loop, again;
	volatile struct _thread_local_storage	*tls = LIBC_TLS();

	// Disable pthread_cancel and set cancel type to deferred
	atomic_set(&tls->__flags, PTHREAD_CANCEL_DISABLE);
	atomic_clr(&tls->__flags, PTHREAD_CANCEL_ASYNCHRONOUS); /* make defered */

	// call the cleanup handlers
	while((handler = tls->__cleanup)) {
		tls->__cleanup = handler->__next;
		handler->__routine(handler->__save);
	}

	// Call the thread_specific_data destructor functions
	for(loop = 0, again = 1; again && loop < PTHREAD_DESTRUCTOR_ITERATIONS; loop++) {
		int										i;
		int										num;

		again = 0;
		num = min(tls->__numkeys, _key_count);
		for(i = 0; i < num; i++) {
			void									*data;
			void									(*destructor)(void *);

			if((destructor = _key_destructor[i]) && (destructor != _KEY_NONE) && (data = tls->__keydata[i])) {
				// Setting the data to NULL was the intention of the POSIX
				// committe, but did not make it into the POSIX spec. Doing
				// this still keeps our implementation POSIX complient, but
				// allow non-POSIX complient applications to be more efficient.
				tls->__keydata[i] = 0;

				destructor(data);
				again = 1;
			}
		}
	}

	// Unlock hanging file mutexes before exiting
	_Unlockfilemtx();

	// Unlock hanging system mutexes before exiting
	_Unlocksysmtx();

	(void)ThreadDestroy_r(0, -1, value_ptr);
#ifdef __GNUC__
	for( ;; ) ; /* Get gcc to shut up about a noreturn function returning */
#endif
}

__SRCVERSION("pthread_exit.c $Rev: 204471 $");
