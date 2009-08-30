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


/*
 * make a copy of the CPU register set in preparation for
 * calling the floating point emulator. Don't trust that the
 * user stack pointer is good.
 */

static void
prep_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned sig) {
	usr_fault(sig, thp, KSP(thp));
	__ker_exit();
}

static const struct fault_handlers prep_fault_handlers = {
	prep_fault, 0
};

uintptr_t rdecl
fpu_emulation_prep(CPU_REGISTERS *src, THREAD *act, int size) {
	uintptr_t		sp = REGSP(src);
	CPU_REGISTERS	*dst;

	STACK_ALLOC(dst, sp, sp, size);

	if(!WITHIN_BOUNDRY(sp, sp, act->aspace_prp->boundry_addr)) {
		usr_fault(MAKE_SIGCODE(SIGSEGV,SEGV_MAPERR,FLTPAGE), act, sp);
		__ker_exit();
	}

	SET_XFER_HANDLER(&prep_fault_handlers);
	*(CPU_REGISTERS *)dst = *src;
	SET_XFER_HANDLER(NULL);

	return sp;
}


/*
 * turn off single stepping while we're in the floating point emulator
 */

int rdecl
begin_fp_emulation(THREAD *act) {
	if(act->flags & _NTO_TF_SSTEP) {
		DEBUG	*dep = act->process->debugger;

		debug_detach_brkpts(dep);
		act->internal_flags |= _NTO_ITF_SSTEP_SUSPEND;
		debug_attach_brkpts(dep);
		return 1;
	}
	return 0;
}

__SRCVERSION("nano_fp_emu.c $Rev: 153052 $");
