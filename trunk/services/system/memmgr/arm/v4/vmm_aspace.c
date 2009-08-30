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

#define	MAX_DOMAIN	16
static ADDRESS	*domain_map[MAX_DOMAIN];
static int		domain_victim = 2;

static void
domain_update(ADDRESS *adp, int domain)
{
	unsigned	base = MVA_BASE(adp->cpu.asid);
	pte_t		*pte = VTOPDIR(base);
	ptp_t		*ptp = VTOL1PT(base);
	ptp_t		acc  = ARM_PTP_L2;
	int			i;

	adp->cpu.domain = domain;
	PTP_DOMAIN_SET(acc, domain);

	/*
	 * Address space is 32MB: 8 "page directories" == 32 L1 entries
	 */
	for (i = 0; i < 8; i++) {
		ptp_t	pt = *pte++;

		if (pt) {
			pt &= ~PGMASK;
			pt |= acc;
			*ptp++ = pt; pt += ARM_L2_SIZE;
			*ptp++ = pt; pt += ARM_L2_SIZE;
			*ptp++ = pt; pt += ARM_L2_SIZE;
			*ptp++ = pt;
		}
		else {
			ptp += 4;
		}
	}

	/*
	 * Update domain in L1 entries for protected global mappings
	 */
	arm_gbl_update(adp, 1);
}

static void
domain_alloc(ADDRESS *adp)
{
	ADDRESS	*old;
	int		i;

	InterruptDisable();
	for (i = 2; i < MAX_DOMAIN; i++) {
		if (domain_map[i] != 0) {
			continue;
		}
		domain_map[i] = adp;
		InterruptEnable();

		/*
		 * Update our L1 page table entries
		 */
		domain_update(adp, i);
		arm_v4_idtlb_flush();
		return;
	}

	/*
	 * Need to steal a domain
	 */
	i = domain_victim;
	if (++domain_victim == MAX_DOMAIN) {
		domain_victim = 2;
	}
	old = domain_map[i];
	domain_map[i] = adp;
		InterruptEnable();

	if (old) {
		domain_update(old, 1);
	}
	domain_update(adp, i);
	arm_v4_idtlb_flush();
}

void
domain_free(ADDRESS *adp)
{
	if (domain_map[adp->cpu.domain] != adp)
		crash();

	/*
	 * WARNING: assumes store is atomic wrt interrupts
	 */
	domain_map[adp->cpu.domain] = 0;
}

void
vmm_aspace(PROCESS *actprp, PROCESS **pactprp)
{
	extern struct cpupage_entry	*_cpupage_ptr;
	ADDRESS	*adp;

	CRASHCHECK(actprp == NULL);
	if ((adp = actprp->memory) != 0) {
		if (_cpupage_ptr->state == 0) {
			if (adp->cpu.domain == 1) {
				/*
				 * Allocate/steal domain for this address space
				 */
				domain_alloc(adp);
			}
			else if (adp->cpu.gbl_swtch) {
				/*
				 * Set our domain in L1 entries for protected global mappings
				 */
				arm_gbl_update(adp, 0);
				arm_v4_idtlb_flush();
			}
			mmu_set_domain(adp->cpu.domain);
		}
		InterruptDisable();

		arm_v4_fcse_set(MVA_BASE(adp->cpu.asid));

		*pactprp = actprp;
		InterruptEnable();
	}
}

__SRCVERSION("vmm_aspace.c $Rev: 153052 $");
