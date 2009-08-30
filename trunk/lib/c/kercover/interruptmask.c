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
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <errno.h>
#include "cpucfg.h"

extern int __interruptmask(int, int);

int InterruptMask(int intr, int id)
{
	int		status;

	if( in_interrupt() ) {
		status = SYSPAGE_ENTRY(callin)->interrupt_mask(intr, id);
		if( status == -1 ) errno = EINVAL;
	} else {
		status = __interruptmask(intr, id);
	}
	return( status );
}

__SRCVERSION("interruptmask.c $Rev: 153052 $");
