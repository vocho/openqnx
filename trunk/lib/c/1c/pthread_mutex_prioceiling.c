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
#include <pthread.h>
#include <sys/neutrino.h>

int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling) {
	while((prioceiling = SyncCtl_r(_NTO_SCTL_SETPRIOCEILING, mutex, &prioceiling)) < 0) {
		if(prioceiling != -EINTR) {
			return -prioceiling;
		}
	}
	if(old_ceiling) {
		*old_ceiling = prioceiling;
	}
	return EOK;
}

int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling) {
	int				status;

	while((status = SyncCtl_r(_NTO_SCTL_GETPRIOCEILING, (sync_t *)mutex, prioceiling)) != EOK) {
		if(status != -EINTR) {
			return -status;
		}
	}
	return EOK;
}


__SRCVERSION("pthread_mutex_prioceiling.c $Rev: 153052 $");
