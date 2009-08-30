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




#include <inttypes.h>
#include <errno.h>

extern unsigned (*_emulator_callout)(unsigned sigcode, void **pdata, void *regs);

extern void SignalFault(unsigned sigcode, void *regs, uintptr_t refaddr);

void
_math_emu_stub(unsigned sigcode, void **pdata, void *regs) {
	int saved_errno;
	unsigned ret;

	saved_errno = errno;
	ret = _emulator_callout(sigcode, pdata, regs);
	errno = saved_errno;

	SignalFault(ret, regs, 0);
}

__SRCVERSION("_math_emu_stub.c $Rev: 153052 $");
