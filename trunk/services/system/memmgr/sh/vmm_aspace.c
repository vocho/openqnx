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

#ifdef	VARIANT_smp
struct intrspin	asid_spin;
#endif

/*
* alloc_asid:
*
* Assign an asid to a process- may steal one from
* another process. Since this code is called from
* vmm_map_address when switching in a process
* we are already in the kernel and there is no need
* for KerextAmInKernel checks.
*
* opt: 1 -- alloc, -1 -- free
*/
// NIY: think about real int safe
static void 
alloc_asid(ADDRESS *adp) {
	static int	asid_rotor = 1;
	int 		asid, i;
	ADDRESS		*oldadp;

	/*
	 * Be very, very careful in here. We can be allocating an asid for
	 * somebody and have an interrupt go off. If the interrupt has a
	 * handler routine, we will re-enterantly execute this code to set up
	 * the address space for the routine.
	 */

	/*
	 * Do a quick scan through the asid_map and see 
	 * if there are any unallocated entries. The
	 * reason why we do this is because it is cheaper 
	 * to do this one scan than to steal an asid,
	 * do a MemPageFlushAsid, and also possibly incur
	 * tlb refills for the poor guy we stole from.
	 * If there are no unallocated asids, then go 
	 * back to where we were in the asid_map.
	 */
	// asid 0 is reserved for system address space
	for(i = 1; i <= VM_ASID_BOUNDARY; ++i) {
		if(asid_map[i] == NULL) {
			asid_map[i] = adp;
			adp->cpu.asid = i;
#ifdef	VARIANT_smp
			/*
			 * Indicate ASID needs purging on all cpus
			 */
			atomic_set(&adp->cpu.asid_flush, LEGAL_CPU_BITMASK);
#endif
			return; 
		}
	}
	/* have to steal one */
	asid = asid_rotor;
	if(++asid_rotor > VM_ASID_BOUNDARY) {
		asid_rotor = 1;
	}
	adp->cpu.asid = asid;
#ifdef	VARIANT_smp
	/*
	 * Indicate ASID needs purging on all cpus
	 */
	atomic_set(&adp->cpu.asid_flush, LEGAL_CPU_BITMASK);
#endif
	oldadp = asid_map[asid];
	asid_map[asid] = adp;
	if(oldadp != NULL) {
		/* 
		 * Mark his asid as invalid so
		 * that when we switch to him he'll
		 * pick up another one.
		 */
		oldadp->cpu.asid = ~0;
#ifdef	VARIANT_smp
		/*
		 * We will flush the asid in vmm_aspace
		 */
#else
		tlb_flush_asid(asid);
#endif
	}
}


void 
vmm_aspace(PROCESS *actprp, PROCESS **pactprp) {
	ADDRESS					*adp;

	if((adp = actprp->memory)) {
		InterruptDisable();
		SPINLOCK(&asid_spin);
		if(adp->cpu.asid > VM_ASID_BOUNDARY) {
			// later move it out for minimize int disable time
			alloc_asid(adp); 
		}
		SPINUNLOCK(&asid_spin);
		smp_tlb_sync(actprp);

		// Set the ASID
		out32(SH_MMR_CCN_PTEH, 
				(in32(SH_MMR_CCN_PTEH) & ~VM_ASID_MASK)  | adp->cpu.asid);
		// Set the page table
		out32(SH_MMR_CCN_TTB, (uintptr_t)adp->cpu.pgdir);
		*pactprp = actprp;

		InterruptEnable();
	}
}

__SRCVERSION("vmm_aspace.c $Rev: 160643 $");
