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
#include <sys/neutrino.h>
#include "cpucfg.h"

int
pthread_rwlock_unlock(pthread_rwlock_t *l) {
	int status, altstat = EOK, id = LIBC_TLS()->__owner;

	if ((status = pthread_mutex_lock(&l->__lock)) == EOK) {

		// determine if the lock is read or write locked and unlock it
		if (l->__active == -1) {
			// some thread has a write lock
			if (l->__owner == id) {
				// this thread is the writer
				l->__active = 0;
				l->__heavy = (l->__blockedreaders > 0);
				l->__owner = -2U;
			} else {
				// this thread does not own the writer lock
				status = EPERM;
			}
		} else if (l->__active > 0) {
			--l->__active;			// I am a reader

			// if the heavy flag is set, see if it can be cleared
			l->__heavy = (l->__heavy) ? (l->__blockedreaders > 0) : l->__heavy;
		} else {
			// no threads have any read/write lock active
			status = EPERM;
		}

		// determine if we should signal a writer or broadcast to readers
		if ((l->__blockedwriters != 0) && (l->__active == 0) && !(l->__heavy)) {
			altstat = pthread_cond_signal(&l->__wcond);
			}
		else if (l->__blockedreaders != 0) {
			altstat = pthread_cond_broadcast(&l->__rcond);
		}

		pthread_mutex_unlock(&l->__lock);
	}

	return (status) ? status : altstat;
}

__SRCVERSION("pthread_rwlock_unlock.c $Rev: 153052 $");
