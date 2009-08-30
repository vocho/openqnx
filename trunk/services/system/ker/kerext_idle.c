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

#include "externs.h"

void
kerext_idle(void *data) {
	INTERRUPT		*itp = data;
	THREAD			*act = actives[KERNCPU];
	uint64_t		tspec;

	timer_next(&tspec);

	if((itp != NULL) && (itp->handler != NULL) && (RUNCPU == 0)) {
		THREAD	*thp;

		thp = itp->thread;
		if(thp->aspace_prp != NULL && thp->aspace_prp != aspaces_prp[KERNCPU]) {
			/*
			 * FIXME: some implementations require the kernel locked
			 */
			lock_kernel();
			memmgr.aspace(thp->aspace_prp, &aspaces_prp[KERNCPU]);
		}
		hook_idle(&tspec, qtimeptr, itp);
	} else {
		halt();
	}

	lock_kernel();
	SETKSTATUS(act,0);
}

__SRCVERSION("kerext_idle.c $Rev: 153052 $");
