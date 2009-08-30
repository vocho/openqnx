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




#include <signal.h>


void (*signal(int signo, void (*func)(int)))(int) {
	struct sigaction act;

	act.sa_sigaction = act.sa_handler = func;
	act.sa_mask.__bits[0] = act.sa_mask.__bits[1] = 0;
	act.sa_flags = 0;

    return(sigaction(signo, &act, &act) ? SIG_ERR : (void (*)())act.sa_handler);
	}

__SRCVERSION("signal.c $Rev: 153052 $");
