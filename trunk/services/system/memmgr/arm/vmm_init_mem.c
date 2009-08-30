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

#include "vmm.h"

paddr_t		cpu_system_paddr_start;
paddr_t		cpu_system_paddr_end;
unsigned	cpu_1to1_vaddr_bias;
int			cpu_1to1_present;
pte_t		*L2_vaddr;
unsigned	l2_prot;

/*
 * Initialise the 1to1 mapping info from the asinfo "1to1" entry
 */
static int
cpu_1to1_init(struct asinfo_entry *as, char *name, void *d)
{
	CRASHCHECK((*KTOL1SC(ARM_1TO1_BASE) & ARM_PTP_VALID) == 0);

	cpu_system_paddr_start = as->start;
	cpu_system_paddr_end = as->end;
	cpu_1to1_vaddr_bias = ARM_1TO1_BASE - cpu_system_paddr_start;
	cpu_1to1_present = 1;
	return 0;
}


/*
 * Unmap 1-1 mapping created by startup to enable MMU
 */
static void
unmap_startup()
{
	unsigned	startup_start;
	unsigned	startup_end;
	ptp_t		*ptp;

	startup_start = _syspage_ptr->un.arm.startup_base;
	startup_end = startup_start + _syspage_ptr->un.arm.startup_size;
	ptp = KTOL1SC(startup_start);
	while (startup_start < startup_end) {
		*ptp++ = 0;
		startup_start += 1 << 20;
	}
	arm_v4_idtlb_flush();
}

void
vmm_init_mem(int phase)
{
	switch(phase) {
	case 0:	
		vx_capability_check();

		pgszlist[0] = MEG(1);
		pgszlist[1] = KILO(64);
		pgszlist[2] = __PAGESIZE;

		L1_table = (ptp_t *)_syspage_ptr->un.arm.L1_vaddr;
		L2_vaddr = (pte_t *)_syspage_ptr->un.arm.L2_vaddr;

		/*
		 * Unmap the 1-1 mapping startup built to enable the MMU
		 */
		unmap_startup();

		/*
		 * Get pointer to ARM specific code for pte/tlb management
		 */
		arm_cpu = SYSPAGE_CPU_ENTRY(arm, cpu);
		l2_prot = arm_cpu->kpte_rw & ~arm_cpu->mask_nc;

		/*
		 * Set up info for the 1-1 mapping area
		 */
		if (walk_asinfo("1to1", cpu_1to1_init, 0) != 0) {
			/*
			 * No 1-1 mapping - let the system restriction list go from [0, ~0]
			 */
			if (ker_verbose) {
				kprintf("startup did not map 1-1 RAM area\n");
			}
			cpu_system_paddr_end = ~0;
		}

		/*
		 * Initialise physical allocator with required number of cache colours
		 */
		pa_init(VX_CACHE_COLOURS());

		/*
		 * Set up msgpass xfer slots
		 */
		VX_XFER_INIT();
		break;

	case 1:
		pa_start_background();
		fault_init();
		lock_init();
		GBL_VADDR_INIT();
		break;

	case 2:
		/*
		 * Called by init_smp() to tear down startup mapping for this cpu
		 */
		unmap_startup();
		break;
	default: break;
	}
}

__SRCVERSION("vmm_init_mem.c $Rev: 164196 $");
