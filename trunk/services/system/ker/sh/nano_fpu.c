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
#include <sh/context.h>


void
fpusave_alloc()
{
	THREAD	*act = actives[KERNCPU];

	if(act->fpudata) crash();
	if((act->fpudata = object_alloc(NULL, &fpu_souls)) == NULL) {
		usr_fault(SIGFPE + (FPE_NOMEM*256) + (FLTFPE*65536),act,KIP(act));
		return;
	}
	/* default fpu exc setting.  A couple of notes here: we normally don't
     * use FPU exceptions, but in the case of SH we have to, because we use
     * the FPU for integer division.  Without enabling divide-by-zero on the
     * FPU, we wouldn't catch integer divide-by-zero.  Also note that
     * whatever we do here will be promptly overwritten if the application
     * is compiled with gcc -- in the startup code in lib/c/startup/sh/crt1.S
     * the first thing we do in the startup code is replace the fpscr.  Still,
     * we'll set it up here properly so we don't have to make any assumptions
     * about the process.
     */
	act->fpudata->fpscr = SH_FPSCR_PR | SH_FPSCR_DN | SH_FPSCR_ENABLE_Z;
}

__SRCVERSION("nano_fpu.c $Rev: 153052 $");
