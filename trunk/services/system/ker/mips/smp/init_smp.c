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

extern void r4k_purge_icache_full(void);

void
init_smp() {

	// If we come in here, we've got more than one CPU.
	nohalt = 1;
	send_ipi_rtn = SYSPAGE_ENTRY(smp)->send_ipi;

	//proc threads have to reference the user copy of the cpu page so
	//that they get the right version for the CPU that they're currently
	//running on.
	_cpupage_ptr = privateptr->user_cpupageptr;

	// Have to flush icache in case the interrupt entry sequence gets
	// allocated exactly at the point where the cpu_startnext() function
	// was located in ram.

	// KLUDGE: use the procnto memmgr code to do the purge since
	// CacheControl() won't work here - the am_inkernel() will return
	// FALSE even though we are in the kernel because we're running
	// on the startup stack and the in_kernel stack range has already
	// been switched to the running values.
	r4k_purge_icache_full();

	cpunum = get_cpunum();

	// This will return through the kernel and idle will be running!
	ker_start();
}

__SRCVERSION("init_smp.c $Rev: 153052 $");
