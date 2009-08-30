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

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
	return SyncTypeCreate_r(_NTO_SYNC_COND, (sync_t *)cond, attr);
}

__SRCVERSION("pthread_cond_init.c $Rev: 153052 $");
