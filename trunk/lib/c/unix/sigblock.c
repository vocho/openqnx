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




#include <unix.h>
#include <signal.h>

/*  here we touch the 'private' members of the sigset_t */
/*  if sigset_t changes, these will also need to change */

int sigblock(int mask) {
	sigset_t m;

	m.__bits[0] = (long)mask;
	m.__bits[1] = 0L;
    sigprocmask(SIG_BLOCK, &m, &m);
    return (int)m.__bits[0];
}

int sigpause(int mask) {
	sigset_t m;

	m.__bits[0] = (long)mask;
	m.__bits[1] = 0L;
    return sigsuspend(&m);
}

int sigsetmask(int mask) {
	sigset_t m;

	m.__bits[0] = (long)mask;
	m.__bits[1] = 0L;
    sigprocmask(SIG_SETMASK, &m, &m);
    return (int)m.__bits[0];
}

int sigunblock(int mask) {
	sigset_t m;

	m.__bits[0] = (long)mask;
	m.__bits[1] = 0L;
    sigprocmask(SIG_UNBLOCK, &m, &m);
    return (int)m.__bits[0];
}

__SRCVERSION("sigblock.c $Rev: 153052 $");
