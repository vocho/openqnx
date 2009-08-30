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




#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/procmsg.h>
#define EXT
#include "pthread_fork.h"

pid_t _fork(unsigned flags, uintptr_t frame) {
	struct _pthread_atfork_func	*f;
	proc_fork_t					msg;
	pid_t						pid;

	if(flags & _FORK_ASPACE) {
		pthread_mutex_lock(&pthread_atfork_mutex);
		for(f = _pthread_atfork_prepare; f; f = f->next) {
			f->func();
		}
	}
	msg.i.type = _PROC_FORK;
	msg.i.zero = 0;
	msg.i.flags = flags;
	msg.i.frame = frame;
	pid = MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, 0, 0);
	if(flags & _FORK_ASPACE) {
		if(pid) {
			for(f = _pthread_atfork_parent; f; f = f->next) {
				f->func();
			}
			pthread_mutex_unlock(&pthread_atfork_mutex);
		} else {
			static const pthread_mutex_t				mutex = PTHREAD_MUTEX_INITIALIZER;
   	
			for(f = _pthread_atfork_child; f; f = f->next) {
				f->func();
			}
			pthread_atfork_mutex = mutex;
		}
	}
	return pid;
}

pid_t fork(void) {
#if 1 	// @@@ TEMPORARY until libc is fork safe wrt mutexs
	extern int          _Multi_threaded;

	if(_Multi_threaded) {
		errno = ENOSYS;
		return -1;
	}
#endif
	return _fork(_FORK_ASPACE, 0);
}

__SRCVERSION("fork.c $Rev: 153052 $");
