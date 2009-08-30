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
#include <stdio.h>
#include <process.h>
#include <signal.h>
#include <pthread.h>

/* POSIX 1003.1 abort   8.2.3.12 */
void abort(void) {
	sigset_t			mask;
	struct sigaction	action;

	/* Make sure caller hasn't ignored SIGABRT */
	sigaction(SIGABRT, 0, &action);
	if(action.sa_handler == SIG_IGN) {
		action.sa_handler = SIG_DFL;
		sigaction(SIGABRT, &action, 0);
	}

	if(action.sa_handler == SIG_DFL) {
		/* Write any dirty streams */
		fflush(0);
	}

	/* Make sure caller hasn't blocked SIGABRT */
	sigfillset(&mask);
	sigdelset(&mask, SIGABRT);
	pthread_sigmask(SIG_SETMASK, &mask, 0);

	/* Do it... */
	raise(SIGABRT);

	/* Caller must have had SIGABRT handler, flush again */
	fflush(0);

	/* Remove the handler so we can try again */
	action.sa_handler = SIG_DFL;
	sigaction(SIGABRT, &action, 0);
	pthread_sigmask(SIG_SETMASK, &mask, 0);

	/* This time it will succeed */
	raise(SIGABRT);

	/* Shouldn't be able to get here, but just incase */
	_exit(EXIT_FAILURE);
}

__SRCVERSION("abort.c $Rev: 153052 $");
