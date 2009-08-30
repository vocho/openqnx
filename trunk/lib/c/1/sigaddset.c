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
#include <errno.h>

int sigaddset(sigset_t *set, int signo)
{
	if ((unsigned long) --signo >= (unsigned long) MAXSIG) {
		errno = EINVAL;
		return(-1);
	}

	#define BITS_SIZE	((unsigned)sizeof(set->__bits[0])*8)

	set->__bits[(unsigned long) signo / BITS_SIZE] |=
			1UL << ((unsigned long) signo % BITS_SIZE);

	return(0);
}

__SRCVERSION("sigaddset.c $Rev: 153052 $");
