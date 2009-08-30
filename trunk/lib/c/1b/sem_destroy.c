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




#include <semaphore.h> 
#include <sys/neutrino.h>
#include <pthread.h>
#include <errno.h>

int
sem_destroy(sem_t *sem) {
	// Force an EINVAL if the semaphore is "statically" initialized.
	if (sem->__owner == _NTO_SYNC_INITIALIZER) {
		errno = EINVAL;
		return(-1);
	}

	return(SyncDestroy((sync_t *) sem));
}

__SRCVERSION("sem_destroy.c $Rev: 153052 $");
