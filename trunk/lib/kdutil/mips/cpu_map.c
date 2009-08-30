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

#include <mips/priv.h>
#include <mips/cpu.h>
#include <sys/mman.h>
#include <sys/kdebug.h>
#include <sys/kdump.h>
#include "kdintl.h"

extern void r4k_update_tlb(uint32_t hi, uint32_t lo0, uint32_t lo1, uint32_t pgmask);

/*
 * TLB macros
 */

struct pte_info {
	unsigned		writeable;
	unsigned		valid;
	unsigned		nocache;
	unsigned		oddbit;
	unsigned		asidmask;
	unsigned		vpnmask;
	unsigned		pfnmask;
};

const struct pte_info r3k_info = {
	MIPS3K_TLB_WRITE,
	MIPS3K_TLB_VALID,
	MIPS3K_TLB_NOCACHE,
	0,
	MIPS3K_TLB_HI_ASIDMASK,
	MIPS3K_TLB_HI_VPNMASK,
	MIPS3K_TLB_LO_PFNMASK,
};

#define MIPS_TLB_LO_PADDR_MASK (MIPS_TLB_LO_PFNMASK | 0xFFFFFF00)

const struct pte_info r4k_info = {
	MIPS_TLB_WRITE,
	MIPS_TLB_VALID,
	MIPS_TLB_UNCACHED << MIPS_TLB_LO_CSHIFT,
	__PAGESIZE,
	MIPS_TLB_HI_ASIDMASK,
	MIPS_TLB_HI_VPN2MASK,
	MIPS_TLB_LO_PADDR_MASK,
};


static unsigned		pt_cacheable_attr;
static unsigned		pfn_topshift;
static unsigned		pgmask_4k;
static const struct pte_info	*info;

void
cpu_init_map(void) {
	if( __cpu_flags & MIPS_CPU_FLAG_NO_WIRED) {
		info = &r3k_info;
	} else {
		info = &r4k_info;
		pt_cacheable_attr = (getcp0_config() & MIPS_CONFIG_KSEG0_COHERENCY) << MIPS_TLB_LO_CSHIFT;
		pgmask_4k = getcp0_pagemask();
	}
	pfn_topshift = SYSPAGE_ENTRY(cpuinfo)[0].flags & MIPS_CPU_FLAG_PFNTOPSHIFT_MASK;
}


static int
colour_match(int actual, int requested) {
	const struct kdump_private	*kdp;

	if(requested == -1) return 1;

	kdp = SYSPAGE_ENTRY(system_private)->kdebug_info->kdump_private;
	if(kdp == NULL) return 1;

	return (actual & *kdp->colour_mask) == requested;
}


#define TEMP_MAP_BASE	(256*1024*1024)

void *
cpu_map(paddr64_t paddr, unsigned size, unsigned prot, int colour, unsigned *mapped_len) {
	unsigned	tlbhi;
	unsigned	tlblo0;
	unsigned	tlblo1;
	uintptr_t	p;
	unsigned	offset;
	unsigned	mapping_size;

	if((paddr < MIPS_R4K_K0SIZE) 
	 && !(prot & PROT_NOCACHE)		
	 && colour_match((_Paddr32t)paddr >> 12, colour)) {
		if(mapped_len != NULL) *mapped_len = size;
		return (void *)MIPS_PHYS_TO_KSEG0(paddr);
	}

	//ZZZ Use bigger pages when possible...
	mmu_needs_restoring = 1;

	if(colour < 0) colour = 0;
	p = TEMP_MAP_BASE + (colour << 12);

	offset = paddr & (__PAGESIZE-1);
	paddr -= offset;
	tlblo0 = info->writeable | info->valid
					| ((unsigned)(paddr >> pfn_topshift) & info->pfnmask);
	if(prot & PROT_NOCACHE) {
		tlblo0 |= info->nocache;
	} else {
		tlblo0 |= pt_cacheable_attr;
	}
	tlblo1 = 0;
	if(p & info->oddbit) {
		tlblo1 = tlblo0;
		tlblo0 = 0;
	}

	tlbhi = ((uintptr_t)p & info->vpnmask) | (getcp0_tlb_hi() & info->asidmask);
		
	r4k_update_tlb(tlbhi, tlblo0, tlblo1, pgmask_4k);

	mapping_size = __PAGESIZE - offset;
	if(size < mapping_size) mapping_size = size;
	if(mapped_len != NULL) *mapped_len = mapping_size;
	return (void *)(p + offset);
}


void
cpu_unmap(void *p, unsigned size) {
	// Nothing to do, r4k_update_tlb() will take care of getting
	// rid of any stale entries
}


unsigned
cpu_vaddrinfo(void *p, paddr64_t *paddrp, unsigned *lenp) {
	//Only needs to handle addresses in the one-to-one mapping area
	if(!MIPS_IS_KSEG0((uintptr_t)p) && !MIPS_IS_KSEG1((uintptr_t)p)) {
		return PROT_NONE;
	}
	*paddrp = MIPS_KSEG0_TO_PHYS(p);
	*lenp = MIPS_R4K_K0SIZE - (unsigned)*paddrp;
	return PROT_READ|PROT_WRITE|PROT_EXEC;
}
