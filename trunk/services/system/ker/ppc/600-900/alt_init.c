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
alt_context_init(THREAD *act) {
	act->cpu.alt->vmx.vscr.u32[0] = 0x00010000; 
	act->cpu.alt->vmx.vscr.u32[1] = 0x00010000; 
	act->cpu.alt->vmx.vscr.u32[2] = 0x00010000; 
	act->cpu.alt->vmx.vscr.u32[3] = 0x00010000; 
}

__SRCVERSION("alt_init.c $Rev: 153052 $");
