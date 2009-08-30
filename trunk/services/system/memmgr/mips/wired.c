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


struct wire_entry {
	uintptr_t			vaddr;
	uint32_t			lo0;
	uint32_t			lo1;
	uint32_t			pm;
};

static unsigned					wire_cookie;
static unsigned					wire_max;


void
wire_sync(ADDRESS *adp) {
	struct wire_entry	*we;
	unsigned			i;
	unsigned			asid;
	unsigned			cpu_bit;
	unsigned			cpu;
	unsigned			flush_check;

	cpu = RUNCPU;
	cpu_bit = 1 << cpu;
	// Turning on the top bit of the index tells r4k_settlb() to look
	// for a matching entry and kill it before loading this one.
	flush_check = (adp->cpu.pending_wire_syncs & cpu_bit) ? 0x80000000 : 0;
	asid = adp->cpu.asid[cpu] << TLB_HI_ASIDSHIFT;
	we = adp->cpu.wires;
	INTR_LOCK(&adp->cpu.slock);
	if(adp->cpu.pending_wire_deletes & cpu_bit) {
		adp->cpu.pending_wire_deletes &= ~cpu_bit;
		i = getcp0_wired();
		for( ;; ) {
			if(i == 0) break;
			--i;
			// Kill the entry by pointing into a set kseg0 location
			r4k_settlb(MIPS_R4K_K0BASE + (i * (__PAGESIZE*2)),
					0, 0, pgmask_4k, i << TLB_IDX_SHIFT);
		}
	}

	i = 0;
	for( ;; ) {
		if(i == wire_max) break;
		if(we->vaddr == VA_INVALID) break;
		r4k_settlb((we->vaddr & VPN_MASK(we->pm)) | asid,
				we->lo0, we->lo1, we->pm, (i  << TLB_IDX_SHIFT) | flush_check);
		++we;
		++i;
	}
	setcp0_wired(i);

	adp->cpu.pending_wire_syncs &= ~cpu_bit;
	INTR_UNLOCK(&adp->cpu.slock);
}

void
wire_check(struct mm_pte_manipulate *data) {
	struct wire_entry	*we;
	unsigned			i;
	uintptr_t			vaddr;
	pte_t				*pt;
	ADDRESS				*adp;
	unsigned			cpu;
	unsigned			mask;
	unsigned			changed = 0;

	adp = data->adp;
	vaddr = data->first & ~(__PAGESIZE*2-1);
	pt = adp->cpu.pgdir[L1IDX(vaddr)];
	if ( pt == NULL ) { /* this can happen if the pagetable entries failed to allocate */
		return;
	}
	pt += L2IDX(vaddr);
	mask = VPN_MASK(pt->pm);

	INTR_LOCK(&adp->cpu.slock);

	// Go through the existing wired entries, and remove any that are
	// a) deleted now
	// b) covered by a new, larger mapping
	// 
	// FIXME: we will also need to be able to remove entries that have
	// now been split.
	//
	for ( i = 0, we = adp->cpu.wires; we->vaddr != VA_INVALID && i < wire_max ; ) {
		if ( ((vaddr & mask) == (we->vaddr & mask)) &&
					((pt[0].pm != we->pm) || (we->lo0 != pt[0].lo) || (we->lo1 != pt[1].lo)) ) {
				adp->cpu.pending_wire_deletes = LEGAL_CPU_BITMASK;
				memmove(&we[0], &we[1], (wire_max - i - 1 ) * sizeof(*we));
				adp->cpu.wires[wire_max - 1].vaddr = VA_INVALID;
				changed = 1;
		} else {
			we++;
			i++;
		}
	}

	if(i < wire_max && ((pt[0].lo | pt[1].lo) & TLB_VALID)) {
		we->lo0 = pt[0].lo;	
		we->lo1 = pt[1].lo;	
		we->pm = pt[0].pm;
		vaddr &= mask;
		// Assign 'vaddr' last so wire_sync() doesn't
		// try to load a partially set up entry.
		we->vaddr = vaddr;
		changed = 1;
	}
	cpu = RUNCPU;
	if( changed ) {
		adp->cpu.pending_wire_syncs = LEGAL_CPU_BITMASK;
	}

	INTR_UNLOCK(&adp->cpu.slock);

	for(i = 0; i < NUM_PROCESSORS; ++i) {
		if(i != cpu) {
			SENDIPI(i, IPI_TLB_FLUSH);
		} else if((getcp0_tlb_hi() & TLB_HI_ASIDMASK) == adp->cpu.asid[i]) {
			wire_sync(adp);
		}
	}
}


void
wire_mcreate(PROCESS *prp) {
	ADDRESS				*adp = prp->memory;
	struct wire_entry	*wires;
	unsigned			i;
	
	adp->cpu.wires = wires = object_to_data(prp, wire_cookie);
	for(i = 0; i < wire_max; ++i) {
		wires[i].vaddr = VA_INVALID;
	}
}


void
wire_init(void) {
#if defined(VARIANT_r3k)
	// The rest of the code uses R4K features, so disable things
	// for R3K's.
	wire_max = 0;
#else
	// Don't allow wired entries to take more than one-eighth of the TLB
	wire_max = num_tlbs/8;
#endif	

	wire_cookie = object_register_data(&process_souls, wire_max*sizeof(struct wire_entry));
}

__SRCVERSION("wired.c $Rev$");
