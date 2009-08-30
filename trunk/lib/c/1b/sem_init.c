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
#include <errno.h>
#include <pthread.h>
#include <sys/neutrino.h>

int
sem_init(sem_t *sem, int pshared, unsigned int value) {
	struct _sync_attr		attr;

	attr.__protocol = (int)value;
	attr.__flags = pshared ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
	attr.__prioceiling = 0;
	
	if(SyncTypeCreate(_NTO_SYNC_SEM, (sync_t *)sem, &attr) == -1) {
		if(errno == EAGAIN) {
			errno = ENOSPC;	// To conform with POSIX 1003.1b
		}
		return(-1);
	}
	return(0);
}

__SRCVERSION("sem_init.c $Rev: 153052 $");
