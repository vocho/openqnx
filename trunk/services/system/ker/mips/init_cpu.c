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

/*
 * init_cpu.c
 *	Set up CPU for Neutrino execution
 *
 */
#include "externs.h"

/*
 * init_cpu()
 *	Initialize machine-dependent memory stuff
 */
void
init_cpu() {

#if defined(VARIANT_32) || defined(VARIANT_r3k)
	//Nothing needed
#elif defined(VARIANT_tx79)
	if(!(__cpu_flags & MIPS_CPU_FLAG_128BIT)) {
		kprintf("Kernel requires 128-bit instruction support.\n");
		crash();
	}
#else
	if(!(__cpu_flags & MIPS_CPU_FLAG_64BIT)) {
		kprintf("Kernel requires 64-bit instruction support.\n");
		crash();
	}
#endif

	cpu_debug_init_perfregs();
}

__SRCVERSION("init_cpu.c $Rev: 153052 $");
