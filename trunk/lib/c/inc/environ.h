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





extern char* _findenv(const char* name, int *offset, int grow);
extern void _unsetenv(const char* name);

#ifdef __QNXNTO__
#include <pthread.h>

#include "pthread_fork.h"

#define _environ_lock()		pthread_mutex_lock(&pthread_atfork_mutex)
#define _environ_unlock()	pthread_mutex_unlock(&pthread_atfork_mutex)

#else
#include <env.h>

#define _environ_lock()		((void)0)
#define _environ_unlock()	((void)0)
#endif


/* __SRCVERSION("environ.h $Rev: 153052 $"); */
