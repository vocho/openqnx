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
#include <semaphore.h> 
#include <mqueue.h> 
#include <devctl.h>
#include <pthread.h>
#include <sys/dcmd_misc.h>

int sem_getvalue(sem_t *sem, int *value) {
	
	// Is it a destroyed semaphore.
	if(sem->__owner == _NTO_SYNC_DESTROYED) {
		errno = EINVAL;
		return -1;
	}

	// Is it a named semaphore.
	if(sem->__owner == _NTO_SYNC_NAMED_SEM) {
		struct mq_attr attr;

		if(_devctl(sem->__count, DCMD_MISC_MQGETATTR, &attr, sizeof attr, 0) == -1) {
			if(errno == EBADF) {
				errno = EINVAL;
			}
			return -1;
		}
		*value = attr.mq_curmsgs;
		return 0;
	}

	*value = sem->__count;
	return 0;
}

__SRCVERSION("sem_getvalue.c $Rev: 153052 $");
