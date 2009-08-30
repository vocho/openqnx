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
#include <sys/mman.h>
#include <sys/neutrino.h>

extern void __my_thread_exit( void * );

const pthread_attr_t pthread_attr_default = {
	PTHREAD_CREATE_JOINABLE |
		PTHREAD_SCOPE_SYSTEM |
		PTHREAD_CANCEL_DEFERRED |
		PTHREAD_CANCEL_ENABLE |
		PTHREAD_INHERIT_SCHED,
	0,							/* No size so kernel will allocate PTHREAD_STACK_MIN	*/
	0,							/* No stackaddr so kernel will allocate one */
	__my_thread_exit,			/* Exit that is always register passing */
	-1,							/* Ignored because of PTHREAD_INHERIT_SCHED */
	{ -1, -1 },					/* Ignored because of PTHREAD_INHERIT_SCHED */
	__PAGESIZE,					/* Default guardpage size should default to PAGESIZE */
	0,							/* Default stack preallocation */
};

__SRCVERSION("pthread_attr_default.c $Rev: 153052 $");
