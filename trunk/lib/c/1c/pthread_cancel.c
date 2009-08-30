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
#include <sys/neutrino.h>

static void canstub( void )
{
	pthread_exit( PTHREAD_CANCELED );
}

int pthread_cancel( pthread_t thread )
{
	return( ThreadCancel_r( thread, canstub ) );
}

// This is an async-cancel safe function

__SRCVERSION("pthread_cancel.c $Rev: 153052 $");
