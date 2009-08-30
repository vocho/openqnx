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
 * tlb_flush_asid()
 *       Flush TLB associated with a particular ASID
 */
static void 
tlb_flush_asid(unsigned asid) {
	struct r4k_tlb	tlb;
	unsigned		x;
	unsigned		idx;

	asid <<= TLB_HI_ASIDSHIFT;
	for(x = 0; x < num_tlbs; ++x) {
		InterruptDisable();
		idx = x << TLB_IDX_SHIFT;
		r4k_gettlb(&tlb, idx);
		if(((tlb.lo0 & TLB_VALID) || (tlb.lo1 & TLB_VALID)) && 
				((tlb.hi & TLB_HI_ASIDMASK) == asid)) {
			// Kill the entry by pointing into a set kseg0 location.
			r4k_settlb(MIPS_R4K_K0BASE + (x * (__PAGESIZE * 2)), 
					0, 0, pgmask_4k, idx);
		}
		InterruptEnable();
	}
}


#if defined(VARIANT_smp)

void 
smp_sync_tlb(PROCESS *prp) {
	ADDRESS     *adp;
	unsigned	mycpu;
	unsigned	mycpu_bit;

	if((adp = prp->memory)) {
		mycpu = RUNCPU;
		mycpu_bit = 1 << mycpu;
		if(adp->cpu.pending_asid_purges & mycpu_bit) {
			tlb_flush_asid(adp->cpu.asid[mycpu]);
			atomic_clr(&adp->cpu.pending_asid_purges, mycpu_bit);
		}
		if(adp->cpu.pending_wire_syncs & mycpu_bit) {
			wire_sync(adp);
		}
	}
}

#else

#define smp_sync_tlb(prp) 

#endif

void 
vmm_aspace(PROCESS *aspaceprp, PROCESS **paspaceprp) {
	ADDRESS     *adp;
	unsigned    *asidp;
	unsigned	mycpu;
	unsigned	mycpu_bit;


	if((adp = aspaceprp->memory)) {
		asidp = &adp->cpu.asid[RUNCPU];

		if(!VALID_ASID(*asidp)) {
			int         asid, i;
			ADDRESS     *oldadp;
			struct asid_mapping *asid_map = gbl_asid_map[RUNCPU];
		
			/*
			 * This poor guy had his asid stolen, go steal someone else's.
			 *
			 * Be very, very careful in here. We can be allocating an asid for
			 * somebody and have an interrupt go off. If the interrupt has a
			 * handler routine, we will re-enterantly execute this code to
			 * set up the address space for the routine.
			 *
			 * Do a quick scan through the asid_map and see 
			 * if there are any unallocated entries. The
			 * reason why we do this is because it is cheaper 
			 * to do this one scan than to steal an asid,
			 * do a tlb_flush_asid, and also possibly incur
			 * tlb refills for the poor guy we stole from.
			 * If there are no unallocated asids, then go 
			 * back to where we were in the asid_map.
			 */
			for (i = 1; i <= TLB_MAX_ASID; ++i) {
				*asidp = i;
				// Do a quick check to see if entry is free before trying to
				// cmpxchg the location (recall silly 41xx is missing ll/sc).
				if (asid_map->map[i] == 0
				  && _smp_cmpxchg((unsigned *)&asid_map->map[i], 0, (unsigned)adp) == 0) {
					goto asid_allocated;
				}
			}
			/* have to steal one */
			asid = asid_map->rotor;
			if (++asid_map->rotor > TLB_MAX_ASID) {
				asid_map->rotor = 1;
			}
			*asidp = asid;
			oldadp = (ADDRESS *)_smp_xchg((unsigned *)&asid_map->map[asid], (unsigned)adp);
			if (oldadp != NULL) {
				/* 
				 * Mark his asid as invalid so
				 * that when we switch to him he'll
				 * pick up another one.
				 */
				if(oldadp != PENDING_PURGE_ADDRESS) {
					oldadp->cpu.asid[RUNCPU] = 0;
				}
				tlb_flush_asid(asid);
				if(icache_flags & CACHE_FLAG_VIRT_TAG) {
					r4k_purge_icache_full();
				}
			}
		}
asid_allocated:
		smp_sync_tlb(aspaceprp);
		InterruptDisable();     // have to set everything atomically
		set_l1pagetable(adp->cpu.pgdir, *asidp << TLB_HI_ASIDSHIFT);
		mycpu = RUNCPU;
		mycpu_bit = 1 << mycpu;
		*paspaceprp = aspaceprp;
		if(adp->cpu.pending_wire_syncs & mycpu_bit) {
			wire_sync(adp);
			/* Note - wire_sync re-enables interrupts */
		}
		InterruptEnable();
	}
}

__SRCVERSION("vmm_aspace.c $Rev: 199588 $");
