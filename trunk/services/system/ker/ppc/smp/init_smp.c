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



// Used when there's only one CPU in the system.
static void 
dummy_send_ipi(struct syspage_entry *sp, unsigned cpu, int cmd, unsigned *ipicmd) {
}

void (*send_ipi_rtn)(struct syspage_entry *, unsigned, int, unsigned *) = dummy_send_ipi;


//
// This code can be removed later, after all the startup's have
// been updated to use the new CPU independent smp section.
//
static void 
old_send_ipi(struct syspage_entry *sp, unsigned cpu, int cmd, unsigned *ipicmd) {
	SYSPAGE_CPU_ENTRY(ppc,smpinfo)->send_ipi(cpu, cmd, &ipicmd[-cpu]);
}

void
init_smp(void) {
	// If we come in here, we've got more than one CPU.

	nohalt = 1;
	if(_syspage_ptr->smp.entry_size > 0) {
		send_ipi_rtn = SYSPAGE_ENTRY(smp)->send_ipi;
	} else {
		//
		// This code can be removed later, after all the startup's have
		// been updated to use the new CPU independent smp section.
		//
		send_ipi_rtn = old_send_ipi;
	}
			
	smp_init_cpu();

	memmgr.init_mem(2);

	_cpupage_ptr = (void *)VM_CPUPAGE_ADDR;
	ker_start();
}

__SRCVERSION("init_smp.c $Rev: 166033 $");
