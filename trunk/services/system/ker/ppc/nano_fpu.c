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
#include <ppc/context.h>

void
fpusave_alloc()
{
	THREAD	*act = actives[KERNCPU];

	if(act->fpudata) crash();

	if(!(__cpu_flags & CPU_FLAG_FPU)) {
		usr_fault(SIGFPE + (FPE_NOFPU*256) + (FLTNOFPU*65536),act,KIP(act));
	}
	act->reg.msr |= PPC_MSR_FP;
	if((act->fpudata = object_alloc(NULL, &fpu_souls)) == NULL) {
		usr_fault(SIGFPE + (FPE_NOMEM*256) + (FLTFPE*65536),act,KIP(act));
		return;
	}

if((unsigned) act->fpudata & 0x0f) crash();

	/* default fpu exc setting */
	/* This is set so that the default values for fp exceptions
	   are the same across all hw platforms and emulators. All
	   floating point exceptions are disabled, round nearest mode.
	 
	act->fpudata->fpscr_val = PPC_FPSCR_VE | PPC_FPSCR_OE | PPC_FPSCR_UE | PPC_FPSCR_ZE;
	*/
	return;
}

__SRCVERSION("nano_fpu.c $Rev: 153052 $");
