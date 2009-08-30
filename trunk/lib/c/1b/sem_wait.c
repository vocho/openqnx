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
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/neutrino.h>

int
sem_wait(sem_t *sem) {

	// Is it a named semaphore.
	if(sem->__owner == _NTO_SYNC_NAMED_SEM) {
		int status;

		if((status = read(sem->__count, NULL, 0)) == -1) {
			if(errno == EBADF) {
				errno = EINVAL;
			}
		}

		return status;
	}

	return(SyncSemWait((sync_t *) sem, 0));
}

__SRCVERSION("sem_wait.c $Rev: 153052 $");
