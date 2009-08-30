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

// Number of entries at the front of the TLB that we _never_ touch
#define WIRE_TLB_NUM_PERM	2

struct wire_entry {
	ppcbke_tlb_t		tlb;
	uint8_t				perm;
};

static unsigned			wire_cookie;
static unsigned			wire_max;
static unsigned			wire_tlb;


void
wire_sync(ADDRESS *adp) {
	struct wire_entry	*we;
	unsigned			cpu_bit = 1 << RUNCPU;
	unsigned			i;

	we = adp->cpu.wires;
	INTR_LOCK(&adp->cpu.slock);
	if(adp->cpu.pending_wire_delete & cpu_bit) {
		ppcbke_tlb_t	dead;

		adp->cpu.pending_wire_delete &= ~cpu_bit;
		memset(&dead, 0, sizeof(dead));
		i = tlbop_get_wired();
		for( ;; ) {
			if(i == WIRE_TLB_NUM_PERM) break;
			--i;
			// Kill the entry 
			tlbop.write_entry(wire_tlb, i, &dead);
		}
	}

	i = 0;
	for( ;; ) {
		if(i == wire_max) break;
		if(we->tlb.epn == VA_INVALID) break;
		we->tlb.tid = get_spr(PPCBKE_SPR_PID);
		tlbop.write_entry(wire_tlb, i + WIRE_TLB_NUM_PERM, &we->tlb);
		++we;
		++i;
	}
	tlbop_set_wired(i + WIRE_TLB_NUM_PERM);
	INTR_UNLOCK(&adp->cpu.slock);
}


static void
wire_aspace(PROCESS *aspaceprp, PROCESS **paspaceprp) {
	vmm_aspace(aspaceprp, paspaceprp);
	wire_sync(aspaceprp->memory);
}


void
wire_check(struct mm_pte_manipulate *data) {
	struct wire_entry	*we;
	unsigned			i;
	uintptr_t			vaddr;
	pte_t				*pt;
	ADDRESS				*adp;
	unsigned			mask;
	ppcbke_tlb_t		new;
	unsigned			cpu;

	adp = data->adp;
	we = adp->cpu.wires;
	vaddr = data->first;
	pt = adp->cpu.pgdir[L1PAGEIDX(vaddr)];
	if ( pt == NULL ) { /* this can happen if the pagetable entries failed to allocate */
		return;
	}
	pt += L2PAGEIDX(vaddr);
	new.ts = 1;
	new.epn = VA_INVALID;
	new.rpn = PTE_PADDR(pt);
	new.attr = (pt->flags & 0x1f) | PPCBKEM_TLB_ATTR_IPROT;
	new.access = (pt->flags >> 5) & 0x7;
	new.access |= new.access << 3;
	new.size = (pt->flags & PPCBKE_PTE_PGSZ_MASK) >> PPCBKE_PTE_PGSZ_SHIFT;
	new.v = PTE_PRESENT(pt) ? 1 : 0;
	if(new.v) {
		mask = ~((0x400 << (new.size*2)) - 1);
	} else {
		mask = ~(__PAGESIZE - 1);
	}
	i = wire_max;
	for( ;; ) {
		if(i == 0) return;
		if(we->tlb.epn == VA_INVALID) break;
		if((vaddr & mask) == (we->tlb.epn & mask)) {
			if(!we->perm && 
			  	((we->tlb.rpn != new.rpn)	
			  ||(we->tlb.attr != new.attr) 
			  ||(we->tlb.access != new.access) 
			  ||(we->tlb.size != new.size) 
			  ||(we->tlb.ts != new.ts) 
			  ||(we->tlb.v != new.v))) {
			  	
				// Something's changed, have to update
				if(!new.v) {
					// Entry deleted, shift stuff down
					--i;
					INTR_LOCK(&adp->cpu.slock);
					adp->cpu.pending_wire_delete = LEGAL_CPU_BITMASK;
					memmove(&we[0], &we[1], i*sizeof(*we));
					we[i].tlb.epn = VA_INVALID;
					INTR_UNLOCK(&adp->cpu.slock);
					goto need_sync;
				}
				break;
			}
			return;
		}
		++we;
		--i;
	}

	if(!new.v) {
		// We don't want to wire unmapped entries.
		return;
	}
	
	memmgr.aspace = wire_aspace;	
	we->perm = (data->shmem_flags & SHMCTL_GLOBAL) ? 1 : 0;
	we->tlb = new;

	MEM_BARRIER_WR();

	// Assign 'epn' last so wire_sync() doesn't
	// try to load a partially set up entry.
	we->tlb.epn = vaddr;

need_sync:	
	cpu = RUNCPU;
	atomic_set(&adp->cpu.pending_wire_sync, LEGAL_CPU_BITMASK & ~(1 << cpu));
	for(i = 0; i < NUM_PROCESSORS; i++) {
		if(i != cpu) {
			SENDIPI(i, IPI_TLB_FLUSH);
		} else if(adp->cpu.asid == get_spr(PPCBKE_SPR_PID)) {
			wire_sync(adp);
		}
	}
}


static int
wire_mcreate(PROCESS *prp) {
	struct wire_entry	*we;
	unsigned			i;
	int					r;
	
	r =  vmm_mcreate(prp);
	if(r == EOK) {
		prp->memory->cpu.wires = we = object_to_data(prp, wire_cookie);
		for(i = 0; i < wire_max; ++i, ++we) {
			we->tlb.epn = VA_INVALID;
		}
	}
	return r;
}


void
wire_init(void) {
	unsigned					num;
	unsigned					i;
	ppcbke_tlb_t				tlb;

	memmgr.mcreate = wire_mcreate;	

	num = _syspage_ptr->un.ppc.tlbinfo.entry_size / sizeof(struct ppc_tlbinfo_entry);
	for(i = 0; i < num; ++i) {
		// Find the TLB that's being used to hold the covering entry - that's
		// the one we're going to put wired entries in as well.
		tlbop.read_entry(i, 0, &tlb);
		if(tlb.v) {
			if(tlb.epn == 0) {
				wire_tlb = i;
				// Don't let the wired entries take up too much of the TLB.
				wire_max = SYSPAGE_CPU_ENTRY(ppc, tlbinfo)[i].num_entries / 8;
				if(wire_max < 4) wire_max = 4;
				break;
			}
		}
	}

	wire_cookie = object_register_data(&process_souls, wire_max*sizeof(struct wire_entry));
}

__SRCVERSION("wired.c $Rev: 206705 $");
