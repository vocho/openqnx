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
 * Storage used by exception mode sp register used to transition to SVC mode
 */
struct exc_save {
	unsigned	val[4];
};
struct exc_save	und_save[PROCESSORS_MAX];
struct exc_save	abt_save[PROCESSORS_MAX];
struct exc_save	irq_save[PROCESSORS_MAX];
struct exc_save	fiq_save[PROCESSORS_MAX];
extern void		smp_exc_save(void *und_save, void *abt_save, void *irq_save, void *fiq_save);

/*
 * Dummy function used when there is only one cpu
 */
static void
dummy_send_ipi(struct syspage_entry *sp, unsigned cpu, int cmd, unsigned *ipi_cmd)
{
}

void (*send_ipi_rtn)(struct syspage_entry *, unsigned, int, unsigned *) = dummy_send_ipi;

void
init_smp()
{
	unsigned	cpu = RUNCPU;

	/*
	 * Unmap the 1-1 mapping startup built to enable the MMU
	 */
	memmgr.init_mem(2);

	/*
	 * FIXME: need to set nohalt so loop does not halt CPU whilst in
	 *        its Ring0(kerext_idle) code. This will cause other CPUs
	 *        spin waiting for the syscall to return.
	 *
	 *        This wastes power - should try to figure out how to get
	 *        SMP idle to actually use a wait-for-interrupt etc. to
	 *        halt the cpu?
	 */
	nohalt = 1;

	/*
	 * Proc threads must access the user cpupage pointer so they get the
	 * correct cpupage for the CPU they are running on.
	 */
	_cpupage_ptr = privateptr->user_cpupageptr;

	/*
	 * Attach the callout send_ipi routine
	 */
	send_ipi_rtn = SYSPAGE_ENTRY(smp)->send_ipi;

	/*
	 * Set up exception save areas for this cpu
	 */
	smp_exc_save(&und_save[cpu], &abt_save[cpu], &irq_save[cpu], &fiq_save[cpu]);

	/*
	 * Set initial MMU domain register value for first __ker_exit
	 */
	mmu_set_domain(0);

	/*
	 * Return through __ker_exit into this cpu's idle thread
	 */
	ker_start();
}

__SRCVERSION("init_smp.c $Rev: 153052 $");
