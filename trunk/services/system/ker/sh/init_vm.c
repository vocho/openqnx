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
#include <sh/cpu.h>

extern uint32_t __exc_tlb_start[];
extern uint32_t __exc_tlb_end[];
extern uint32_t __exc_tlb_start_a[];
extern uint32_t __exc_tlb_end_a[];


void
copy_vm_code() {

	// If the CPU supports 32-bit mode, it uses the SH4A tlb-miss handler.
	if ( __cpu_flags & SH_CPU_FLAG_32_BIT ) {
		copy_code((void*)((unsigned) _syspage_ptr->un.sh.exceptptr + SH_EXC_TLBMISS),
				__exc_tlb_start_a, __exc_tlb_end_a);
	} else {
		copy_code((void*)((unsigned) _syspage_ptr->un.sh.exceptptr + SH_EXC_TLBMISS),
				__exc_tlb_start, __exc_tlb_end);
	}
}

__SRCVERSION("init_vm.c $Rev: 159814 $");
