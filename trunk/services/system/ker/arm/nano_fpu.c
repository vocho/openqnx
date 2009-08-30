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

static void
dummy_ctx_init(FPU_REGISTERS *fpu)
{
	crash();
}

static void
dummy_ctx_save(FPU_REGISTERS *fpu)
{
	crash();
}

static void
dummy_ctx_restore(FPU_REGISTERS *fpu)
{
	crash();
}

static void
dummy_fpemu_prep(ARM_FPEMU_CONTEXT *ctx)
{
}

void	(*fpu_ctx_init)(FPU_REGISTERS *)		= dummy_ctx_init;
void	(*fpu_ctx_save)(FPU_REGISTERS *)		= dummy_ctx_save;
void	(*fpu_ctx_restore)(FPU_REGISTERS *)		= dummy_ctx_restore;
void	(*fpu_disable)(void *, void *)			= 0;
void	(*fpu_fpemu_prep)(ARM_FPEMU_CONTEXT *)	= dummy_fpemu_prep;

void
fpusave_alloc()
{
	THREAD	*act = actives[KERNCPU];

	if(act->fpudata) crash();

	/*
	 * If no fpu then abort
	 */
    if((__cpu_flags & CPU_FLAG_FPU) == 0) {
		usr_fault(SIGFPE + (FPE_NOFPU*256) + (FLTNOFPU*65536), act, KIP(act));
		return;
	}

	/*
	 * Allocate a save area and initialize it.
	 */
	if((act->fpudata = object_alloc(NULL, &fpu_souls)) == NULL) {
		usr_fault(SIGFPE + (FPE_NOMEM*256) + (FLTNOFPU*65536), act, KIP(act));
		return;
	}
	fpu_ctx_init(act->fpudata);
}

__SRCVERSION("nano_fpu.c $Rev: 153052 $");
