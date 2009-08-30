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




#include <unistd.h>
#include <semaphore.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <pthread.h>
#include <sys/iomsg.h>

int
sem_trywait(sem_t *sem) {

	// Is it a named semaphore.
	if(sem->__owner == _NTO_SYNC_NAMED_SEM) {
		int status;

		if((status = _readx(sem->__count, NULL, 0, _IO_XFLAG_NONBLOCK, NULL, 0)) == -1) {
			if(errno == EBADF) {
				errno = EINVAL;
			}
		}
		return status;
	}

	return(SyncSemWait((sync_t *) sem, 1));
}

__SRCVERSION("sem_trywait.c $Rev: 153052 $");
