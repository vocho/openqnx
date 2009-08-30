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
 * Dummy function used when there is only one cpu
 */
static void
dummy_send_ipi(struct syspage_entry *sp, unsigned cpu, int cmd, unsigned *ipi_cmd)
{
}

void	(*send_ipi_rtn)(struct syspage_entry *, unsigned, int, unsigned *) = dummy_send_ipi;

void
init_smp()
{
	/*
	 * Perform per-cpu memmgr initialisation
	 */
	memmgr.init_mem(2);

	/*
	 * FIXME: need to set nohalt so loop() does not halt the CPU while
	 *        it is in the Ring0(kerext_idle) code.
	 *        Otherwise, other cpus will will spin waiting for the syscall
	 *        to exit.
	 *
	 *        This wastes power, so we should really figure out how to get
	 *        idle to correctly power down individual cpus.
	 */
	nohalt = 1;

	/*
	 * proc threads must access the user cpupage pointer so they get the
	 * correct cpupage for the CPU they are running on.
	 */
	_cpupage_ptr = privateptr->user_cpupageptr;

	/*
	 * Attach the send_ipi callout
	 */
	send_ipi_rtn = SYSPAGE_ENTRY(smp)->send_ipi;

	/*
	 * Return through __ker_exit() into this cpu's idle thread
	 */
	ker_start();
}

__SRCVERSION("init_smp.c: $Rev: 199330 $");
