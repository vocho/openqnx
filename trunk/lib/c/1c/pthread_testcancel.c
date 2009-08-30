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
#include "cpucfg.h"

#undef pthread_testcancel

void
pthread_testcancel( void ) {
	if( ( LIBC_TLS()->__flags & ( PTHREAD_CSTATE_MASK | PTHREAD_CANCEL_PENDING ) ) == ( PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_PENDING ) )
		pthread_exit( PTHREAD_CANCELED );
}

__SRCVERSION("pthread_testcancel.c $Rev: 153052 $");
