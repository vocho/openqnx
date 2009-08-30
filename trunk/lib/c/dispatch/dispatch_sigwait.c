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




#include <sys/dispatch.h>
#include <signal.h>
#include <malloc.h>
#include "dispatch.h"


int sigwait_attach(dispatch_t *dpp, sigwait_attr_t *attr, int signo,
		int (*func)(sigwait_context_t *ctp, int fd, unsigned flags, void *handle),
		void *handle) {

	return 0;
}


int sigwait_detach(dispatch_t *dpp, int signo, int flags) {

	return 0;
}

int _sigwait_handler(dispatch_context_t *ctp) {

	return 0;
}

__SRCVERSION("dispatch_sigwait.c $Rev: 153052 $");
