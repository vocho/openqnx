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

/*
 * Allocate virtual-physical mappings for bootstrapping memmgr.
 */
void *
cpu_early_paddr_to_vaddr(paddr_t p, unsigned size, paddr_t *l2mem)
{
	static unsigned	kpte_rw;
	static unsigned	mask_nc;
	static ptp_t	*L1pt;

	unsigned	va;
	unsigned	pa;
	unsigned	fva = 0;

	if (CPU_1TO1_IS_PADDR(p)) {
		return (void *)((uintptr_t)p + CPU_1TO1_VADDR_BIAS);
	}

	if (kpte_rw == 0) {
		kpte_rw = SYSPAGE_CPU_ENTRY(arm, cpu)->kpte_rw;
		mask_nc = SYSPAGE_CPU_ENTRY(arm, cpu)->mask_nc;
		L1pt = (ptp_t *)_syspage_ptr->un.arm.L1_vaddr;
	}

	if ((fva = cpu_sysvaddr_find(va_rover, size)) == VA_INVALID) {
		crash();
	}
	va_rover = fva + size;

	for (va = fva; va < va_rover; va += __PAGESIZE, p += __PAGESIZE) {
		/*
		 * Check if we need to allocate a page table for va
		 *
		 * FIXME_smp: SMP needs smp_pde_set?
		 */
		if (*KTOPDIR(va) == 0) {
			ptp_t	*ptp = L1pt + ((va >> 20) & ~3);
			int		i;

			*l2mem -= __PAGESIZE;
			pa = *l2mem + 1;	// +1 because l2mem points to _last_ free paddr

			/*
			 * FIXME_smp: SMP needs smp_pde_set?
			 */
			*KTOPDIR(va) = (pa | kpte_rw) & ~mask_nc;
			for (i = 0, pa |= ARM_PTP_L2; i < 4; i++, pa += ARM_L2_SIZE) {
				*ptp++ = pa;
			}
			CPU_ZERO_PAGE(KTOPTP(va), __PAGESIZE, 0);
		}
		*KTOPTEP(va) = (p | kpte_rw);
	}

	return (void *)fva;
}

unsigned
cpu_whitewash(struct pa_quantum *pq)
{
	/*
	 * FIXME: zero the quantum
	 */
	return 0;
}

__SRCVERSION("cpu_pa.c $Rev: 153052 $");
