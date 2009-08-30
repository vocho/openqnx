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
*	ker/arm/init_cpu.c


*/

#include "externs.h"

void
init_cpu()
{
	/*
	 * Set initial MMU domain register value for first __ker_exit
	 */
	mmu_set_domain(0);

	/*
	 * Enable data abort fixup if necessary
	 */
	switch (arm_cpuid() & 0xfff0) {
	case 0x7200:
		mmu_abort = arm_abort_fixup;
		break;

	default:
		mmu_abort = 0;
		break;
	}

	/*
	 * Force a reference to ClockCycles() to ensure it is linked into
	 * the regular procnto from the libnto.a archive.
	 * This is required for the APS scheduler module that can be linked
	 * in at mkifs time.
	 */
	(void)ClockCycles();

	/*
	 * Attach VFP support if VFP h/w is present.
	 */
	if (!fpuemul && (__cpu_flags & CPU_FLAG_FPU) != 0) {
		vfp_init();
	}
}

__SRCVERSION("init_cpu.c $Rev: 153052 $");
