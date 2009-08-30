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
kerext_v86_enter(void *data) {
	int					swi = (int)data;
	struct _v86_memory	*vp = (void *)realmode_addr;
	THREAD				*act = actives[KERNCPU];

	if(!kerisroot(act)) {
		return;
	}

	// Setup a stack and point eip to stub entry point.
	vp->reg.cs = vp->reg.ss = 0;
	vp->reg.esp = (uintptr_t)&vp->stack[sizeof(vp->stack)] - realmode_addr;
	vp->reg.eip = (uintptr_t)vp->stubcode - realmode_addr;
	vp->stubcode[0] = 0xcd;					// "int" opcode
	vp->stubcode[1] = (char) swi;
	vp->stubcode[2] = 0xcf;					// "iret" opcode
//memcpy(vp->stubcode, "\xfb\xfa\xef\xe4\x20\xcf", 6);	// Test code
//memcpy(vp->stubcode, "\xfb\xfa\xef\xe4\x20\xcf", 6);	// Test code
//memcpy(vp->stubcode, "\xfa\xfa\xcf", 3);				// Test code

	// Set V86 flag, IOPL 1, intr enabled
	vp->reg.efl = 0x21200;

	lock_kernel();
	act->flags |= _NTO_TF_V86;
	SETKSTATUS(act, 0);
}

__SRCVERSION("kerext_v86.c $Rev: 153052 $");
