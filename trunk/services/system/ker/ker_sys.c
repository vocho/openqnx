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

int kdecl
ker_sys_cpupage_get(THREAD *act, struct kerargs_sys_cpupage_get *kap) {
	intptr_t			status;

	if(kap->index == CPUPAGE_ADDR) {
		status = (intptr_t)privateptr->user_cpupageptr;
	} else if(kap->index >= 0 && kap->index < CPUPAGE_MAX) {
		intptr_t	*cpupage = (intptr_t *)cpupageptr[KERNCPU];

		status = cpupage[kap->index];
	} else {
		return EINVAL;
	}
	lock_kernel();
	SETKSTATUS(act, status);
	return ENOERROR;
}

int kdecl
ker_sys_cpupage_set(THREAD *act, struct kerargs_sys_cpupage_set *kap) {
	PROCESS					*prp;

	if(kap->index != CPUPAGE_PLS) {
		return EINVAL;
	}

	lock_kernel();
	act->process->pls = (void *)kap->value;
	
	// Force a reload of PLS
	prp = actives_prp[KERNCPU];
	if(prp->debugger && debug_detach_brkpts) {
		debug_detach_brkpts(prp->debugger);
	}
	//RUSH3: If we instead assign "sysmgr_prp", we can probably remove
	//RUSH3: NULL pointer tests in the kernel.S files when they check
	//RUSH3: to see if they have to remove breakpoints from the old
	//RUSH3: process in the ker_exit code.
	actives_prp[KERNCPU] = 0;

	return EOK;
}

__SRCVERSION("ker_sys.c $Rev: 153052 $");
