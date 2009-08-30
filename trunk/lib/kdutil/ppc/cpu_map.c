/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <sys/mman.h>
#include <ppc/cpu.h>
#include <ppc/inline.h>
#include <ppc/bookecpu.h>
#include <kernel/nto.h>
#include "kdintl.h"

struct pte64 {
	uint64_t	hi;
	uint64_t	lo;
};

static struct pte64 	save_pte;
static struct pte64		*save_pte_loc;
static int				covering_tlb = -1;

#define CALC_HASH64(vaddr, large, avpn, pte1, pte2) 				\
	{																\
		uint32_t	hash1, hash2;									\
		uint32_t	hash1_lo, hash1_hi, hash2_lo, hash2_hi;			\
		uintptr_t	hash_base;										\
		unsigned	hash_mask;										\
		unsigned	vpn;											\
																	\
		if(large) {													\
			vpn = ((vaddr) >> 24) & 0x000ff;						\
			hash1 = (vpn >> 4) ^ (vpn & 0xf);						\
		} else {													\
			vpn = ((vaddr) >> 12) & 0xfffff;						\
			hash1 = (vpn >> 16) ^ (vpn & 0xffff);					\
		}															\
		(avpn) = (vaddr) >> 23;										\
																	\
		hash2 = ~hash1;												\
																	\
		hash1_lo = (hash1 & 0x000007ff) << 7;						\
		hash1_hi = (hash1 & 0x0007f800) >> 11;						\
																	\
		hash2_lo = (hash2 & 0x000007ff) << 7;						\
		hash2_hi = (hash2 & 0x0007f800) >> 11;						\
																	\
		hash_mask = (1 << (sdr1 & 0x1f)) - 1;						\
		hash1_hi = (hash1_hi & hash_mask) << 18;					\
		hash2_hi = (hash2_hi & hash_mask) << 18;					\
																	\
		hash_base = (uintptr_t)(sdr1 & ~((1 << 18) - 1));\
		pte1 = (struct pte64 *)(hash_base | hash1_lo | hash1_hi);	\
		pte2 = (struct pte64 *)(hash_base | hash2_lo | hash2_hi);	\
	}


void
cpu_init_map(void) {
	// Nothing to do
}


// Use the message pass area for the temp mapping vaddr
#define MAP_TEMP_VADDR	VM_MSG_XFER_START

#define PADDR_TO_BAT(p)	((p & 0xfffff000) | ((p>>30) & 0x04) | ((p>>24)&0x00000e00))

void *
cpu_map(paddr64_t paddr, unsigned size, unsigned prot, int colour, unsigned *mapped_len) {
	paddr_t					base;
	unsigned				offset;
	unsigned				mapping_size;
	struct cpu_extra_state	*state;
	unsigned				i;
	unsigned				j;
	unsigned				num_entries;
	ppcbke_tlb_t			*tlb;
	ppcbke_tlb_t			temp_tlb;
	unsigned				avpn;
	struct pte64			*pte1;
	struct pte64			*pte2;
	unsigned				sdr1;
	unsigned				lo;

	if((paddr < MEG(256)) && !(prot & PROT_NOCACHE)) {
		if(mapped_len != NULL) *mapped_len = size;
		return (void *)(uintptr_t)paddr;
	}
	if(private->kdebug_info == NULL) {
		// Can only reference 1-to-1 area before procnto is up.
		return NULL;
	}
	state = private->kdebug_call->extra;
	mmu_needs_restoring = 1;
	// The upper levels arrange that they only have one mapping active
	// at a time once procnto is up, so that simplifies the code here.
	switch(SYSPAGE_CPU_ENTRY(ppc, kerinfo)->ppc_family) {
	case PPC_FAMILY_400:	
	case PPC_FAMILY_800:	
	default:	
		//ZZZ handle 400/800 mapping above one-to-one area
		return NULL;
	case PPC_FAMILY_booke:	

		i = 0;
		for(;;) {
			num_entries = state->u.ppcbke.tlb[i].num_entries;
			if(num_entries == 0) break;
			for(j = 0; j < num_entries; ++j) {
				tlb = &state->u.ppcbke.tlb[i].entry[j];
				if(tlb->v) {
					if((covering_tlb < 0) 
					  && (tlb->epn == 0)
					  && (tlb->ts == 0)) {
						covering_tlb = i;
					}
					if((mmu_needs_restoring < 2) 
					  && tlb->ts 
					  && (tlb->tid == get_spr(PPCBKE_SPR_PID))
					  && (tlb->epn == MAP_TEMP_VADDR)) {
						// We've got a valid TLB entry overlapping our
						// temp mapping vaddr. Disable it for right now
						tlb->v = 0;
						bke_tlb_write(i, j, tlb);
						tlb->v = 2;
						mmu_needs_restoring = 2;
					}
				}
			}
			++i;
		}
		mapping_size = 0x1000;
		base = paddr & ~(mapping_size - 1);

		// create the temporary mapping
		temp_tlb.rpn = base;
		temp_tlb.epn = MAP_TEMP_VADDR;
		temp_tlb.tid = get_spr(PPCBKE_SPR_PID);
		temp_tlb.attr = PPCBKE_TLB_ATTR_M | PPCBKEM_TLB_ATTR_IPROT;
		if(prot & PROT_NOCACHE) {
			temp_tlb.attr |= PPCBKE_TLB_ATTR_I | PPCBKE_TLB_ATTR_G;
		}
		temp_tlb.access = PPCBKE_TLB_ACCESS_SR;
		if(prot & PROT_WRITE) {	
			temp_tlb.access |= PPCBKE_TLB_ACCESS_SW;
		}
		temp_tlb.size = PPCBKE_TLB_SIZE_4K;
		temp_tlb.ts = (get_msr() & PPC_MSR_IS) ? 1 : 0;
		temp_tlb.v = 1;
		// tlbno==covering_tlb, entryno==0 is a 256M covering entry with TS==0.
		// tlbno==covering_tlb, entryno==1 is a 256M covering entry with TS==1.
		// The first is used for entry into exception handlers, the second
		// for while procnto code is running. Sometimes we're in here using
		// one, sometimes the other. Check the MSR to see which one we're
		// using and temporarily overwrite the other with an entry for the
		// new mapping. The cpu_unmap() function below restores the entry 
		// back to it's original state.
		bke_tlb_write(covering_tlb, (get_msr() & PPC_MSR_IS) ? 0 : 1, &temp_tlb);
		break;
	case PPC_FAMILY_900:
		mapping_size = 0x1000;
		base = paddr & ~0xfff;
		// set up mapping segment 
		ppc_slbmte(0x1000 + PPC64_SLB0_KP|PPC64_SLB0_C, 
				(1 << PPC64_SLB1_ESID_SHIFT) | PPC64_SLB1_V | 1);
		sdr1 = get_spr(PPC_SPR_SDR1);
		CALC_HASH64(MAP_TEMP_VADDR, 0, avpn, pte1, pte2);
		ppc_tlbie(MAP_TEMP_VADDR);
		save_pte_loc = pte1;
		save_pte = *pte1;
		pte1->hi = 0;
		ppc_ptesync();
		pte1->lo = base | (PPC_TLBLO_M|PPC_TLBLO_PP_NA);
		if(prot & PROT_NOCACHE) {
			pte1->lo |= PPC_TLBLO_I | PPC_TLBLO_G;
		}
		pte1->hi = (avpn << PPC64_TLBHI_AVPN_SHIFT) | PPC64_TLBHI_V;
		ppc_tlbsync();
		ppc_sync();
		break;
	case PPC_FAMILY_600:	
		mapping_size = MEG(256);
		base = paddr & ~((paddr_t)MEG(256)-1);
		lo = PADDR_TO_BAT(base) | PPC_BATL_PP_R | PPC_BATL_M;
		if(prot & PROT_NOCACHE) {
			lo |= PPC_BATL_G | PPC_BATL_I;
		}
		set_spr(PPC_SPR_DBAT3L, lo);
		set_spr(PPC_SPR_DBAT3U, PPC_BATU_BEPI(MAP_TEMP_VADDR >> 17) | PPC_BATU_BL_256M | PPC_BATU_VS | PPC_BATU_VP);
		break;
	}
	ppc_sync();
	offset = paddr - base;
	mapping_size -= offset;
	if(mapping_size > size) mapping_size = size;
	if(mapped_len != NULL) *mapped_len = mapping_size;
	return (void *)(MAP_TEMP_VADDR + offset);
}


void
cpu_unmap(void *p, unsigned size) {
	struct cpu_extra_state	*state;
	unsigned				i;
	unsigned				j;
	unsigned				num_entries;
	ppcbke_tlb_t			*tlb;

	if((uintptr_t)p < MAP_TEMP_VADDR) {
		// in one-to-one area, nothing to do
		return;
	}
	state = private->kdebug_call->extra;
	switch(SYSPAGE_CPU_ENTRY(ppc, kerinfo)->ppc_family) {
	case PPC_FAMILY_400:
	case PPC_FAMILY_800:
	default:	
		//ZZZ handle 400/800 unmapping above one-to-one area
		break;
	case PPC_FAMILY_booke:	
		i = (get_msr() & PPC_MSR_IS) ? 0 : 1, 
		bke_tlb_write(covering_tlb, i, &state->u.ppcbke.tlb[0].entry[i]);
		if(mmu_needs_restoring == 2) {
			i = 0;
			for(;;) {
				num_entries = state->u.ppcbke.tlb[i].num_entries;
				if(num_entries == 0) break;
				for(j = 0; j < num_entries; ++j) {
					tlb = &state->u.ppcbke.tlb[i].entry[j];
					if(tlb->v == 2) {
						// Temporarily disabled entry, turn it back on.
						tlb->v = 1;
						bke_tlb_write(i, j, tlb);
						break;
					}
				}
				++i;
			}
			mmu_needs_restoring = 1;
		}
		break;
	case PPC_FAMILY_900:
		*save_pte_loc = save_pte;
		ppc_ptesync();
		ppc_tlbie(p);
		ppc_tlbsync();
		ppc_sync();
		// restore mapping segment 
		ppc_slbmte(state->u.ppc900.slb[1].v, state->u.ppc900.slb[1].e | 1);
		ppc_sync();
		break;
	case PPC_FAMILY_600:	
		set_spr(PPC_SPR_DBAT3L, 0);
		set_spr(PPC_SPR_DBAT3U, 0);
		break;
	}
}


unsigned
cpu_vaddrinfo(void *p, paddr64_t *paddrp, unsigned *lenp) {
	//Only needs to handle addresses in the one-to-one mapping area
	*paddrp = (uintptr_t)p;
	*lenp = MEG(256) - (uintptr_t)p;
	return PROT_READ|PROT_WRITE|PROT_EXEC;
}
