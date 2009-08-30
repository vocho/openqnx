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




#ifndef EXT
#define EXT extern
#define INIT(a)
#else
#define INIT(a) = a
#endif

EXT struct _pthread_atfork_func {
	struct _pthread_atfork_func		*next;
	void							(*func)(void);
} *_pthread_atfork_prepare, *_pthread_atfork_parent, *_pthread_atfork_child;

EXT pthread_mutex_t pthread_atfork_mutex INIT(PTHREAD_MUTEX_INITIALIZER);

/* __SRCVERSION("pthread_fork.h $Rev: 153052 $"); */
