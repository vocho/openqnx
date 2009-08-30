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
KerextLock(void) {
#ifndef NDEBUG
	if(!(get_inkernel() & INKERNEL_NOW)) {
		crash();
	}
#if defined(VARIANT_smp)
	if((RUNCPU != KERNCPU) && (qtimeptr != NULL) && (qtimeptr->nsec_inc != 0)) {
		crash();
	}
#endif
#endif
	lock_kernel();
}

void
KerextUnlock(void) {
	unlock_kernel();
}


int
KerextAmInKernel(void) {
	return(am_inkernel());
}


void
KerextStatus(THREAD *thp, int status) {
	if(!thp) {
		thp = actives[KERNCPU];
	}
	lock_kernel();
	SETKSTATUS(thp, status);
}


int
KerextNeedPreempt(void) {
	return NEED_PREEMPT(actives[RUNCPU]) ? 1 : 0;
}

int
KerextSyncOwner(pid_t pid, int tid) {
	return SYNC_OWNER_BITS(pid, tid-1);
}

__SRCVERSION("kerext_misc.c $Rev: 202687 $");
