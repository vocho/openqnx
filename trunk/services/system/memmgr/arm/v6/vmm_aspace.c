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

void
vmm_aspace(PROCESS *actprp, PROCESS **pactprp)
{
	ADDRESS	*adp;

	CRASHCHECK(actprp == NULL);
	if ((adp = actprp->memory) != 0) {
		pte_t	*pte;
		ptp_t	*ptp;

#ifdef	VARIANT_smp
		/*
		 * Check if we need to flush our ASID
		 */
		if (adp->cpu.asid_flush & (1 << RUNCPU)) {
			/*
			 * Invalidate all (unified) TLB entries for our ASID
			 *
			 * FIXME_v6: current ARM11 and MPcore have unified TLBs
			 *        Need to check for other processors whether the unified op
			 *        will correctly invalidate both I and D TLBs...
			 */
			arm_v6_tlb_asid(adp->cpu.asid);
			atomic_clr(&adp->cpu.asid_flush, 1 << RUNCPU);
		}
#endif

		InterruptDisable();

		if (adp->cpu.l1_pte) {
			/*
			 * Map the 8K L1 table at 0xffff2000
			 * Allows direct L1 descriptor access via UTOL1SC/UTOL1PT macros
			 *
			 * The pte entries are marked nG so translation is tagged with ASID
			 */
			pte = KTOPTEP(ARM_V6_USER_L1);
			pte[0] = adp->cpu.l1_pte;
			pte[1] = adp->cpu.l1_pte + __PAGESIZE;

			/*
			 * Map the user "page directory" page
			 * Allows mapping of user page tables via UTOPDIR macro
			 *
			 * The pte entry is marked nG so translation is tagged with ASID
			 */
			*KTOPDIR(ARM_UPTE_BASE) = adp->cpu.l2_pte;

			/*
			 * Map the user page tables
			 * Allows direct L2 descriptor access via UTOPTEP/UTOPTP macros
			 *
			 * The actual L2 table entries (contained in the user "page directory")
			 * are marked nG, so translations are tagged with ASID
			 */
			ptp = KTOL1SC(ARM_UPTE_BASE);
#ifdef	VARIANT_smp
			/*
			 * KTOL1SC uses cpu0's L1 table, so adjust the pointer to point
			 * to RUNCPU's L1 table
			 */
			ptp += RUNCPU * (ARM_L1_SIZE / sizeof *ptp);
#endif
			ptp[0] = adp->cpu.l2_ptp;
			ptp[1] = adp->cpu.l2_ptp + ARM_L2_SIZE;

			/*
			 * Set TTBR0 to point at user L1 table
			 */
			arm_v6_ttbr0_set(adp->cpu.ttbr0);
		}

/*
 * FIXME_v6: need to set TTBR0 for SYSMGR_PID - eg. to TTBR1 value?
 */

		/*
		 * Set ASID and flush BTAC
		 */
		arm_v6_asid_set(adp->cpu.asid);
		arm_v6_flush_btc();
		arm_v6_dsb();

		*pactprp = actprp;
		InterruptEnable();
	}
}

__SRCVERSION("vmm_aspace.c $Rev: 153052 $");
