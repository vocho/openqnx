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
#include <ppc/700cpu.h>
#include <ppc/603cpu.h>

//YYY: works, but needs cleanup (probably needs "ptesync"'s added)

/*
 * extern declarations for variables that are in the kernel. Really
 * annoying, they should go away when we merge all VM code in a single
 * entity.
 */
extern uint32_t *cpupageptr[];

static VECTOR	asid_vector;

/*
 * Hash table entry
 */

struct htentry {
	uint64_t hthi;
	uint64_t htlo;
};

/*
 * This is the hash table base and mask 
 */

extern struct htentry 		*HashTable_base;
extern uint32_t 			HashTable_mask;
extern intrspin_t			ht_slock;

/*
 * Note: VSID_MAKE_REAL must match what set_l1pagetable() in ker/ppc/900/vm.s
 * does for vsid adjustment to cover the address range.
 */
#define VSID_MAKE_REAL(base, vaddr)		((base) + ((vaddr) >> 28))

#define VSID_SCRAMBLE(asid)		(((asid) << 8) | 0x80)
#define VSID_UNSCRAMBLE(asid)	((asid) >> 8)

#define PPC64_HENTRY_AVPN(vsid,vaddr)  (((vsid) << 5) | ((vaddr)&0x0FFFF000)>>23)
#define PPC64_PRIMARY_HASH(vsid,vaddr) (((vsid) & 0x7ffff) ^ (((vaddr) >> 12) & 0xffff))
#define PPC64_CALC_PTEG(hash)	\
		((struct htentry *) ((uintptr_t)HashTable_base |	\
			(HashTable_mask & (((hash) & 0x7f800) << 7)) |	\
			(((hash) & 0x7ff) << 7)))

// None of the 900's need software to sync the TLB's, so we can get away
// with deleting this
//#define SMP_SYNC_TLBS() if(__cpu_flags & PPC_CPU_SW_TLBSYNC) SMP_FLUSH_TLB()
#define SMP_SYNC_TLBS()


/*
 * I don't think we really need this for the 700, as the TLB's are tagged
 * with the vsid.
 */

#define TLB_INVALID_INC _ONEBIT32B(19)
#define TLB_INVALID_END _ONEBIT32B(12)


/*
 * addr: bit 14 - 19 is effective
 * Note: for SMP, we must add tlbsync ops after to insure consistency
 */


static void inline
fam_pte_flush_entry(unsigned vaddr) {
#ifdef VARIANT_smp
	intrspin_t 		*locked = &ht_slock;
	uint32_t		tmp = 0, msr = 0;

	asm volatile(
		"	mfmsr	%3;"
		"	rlwinm	%2,%3,0,17,15;"
		"	mtmsr	%2;"
		"1:;"
		"	lwarx	%2,0,%1;"
		"	stwcx.	%1,0,%1;"
		"	bne-	1b;"
		"   mr.		%2,%2;"
		"	bne-	1b;"
		"	isync;"
		"	tlbie	%0;"
		"	eieio;"
		"	tlbsync;"
		"	sync;"
		"	li		%2,0;"
		"	stw 	%2,0(%1);"
		"	sync;"
		"	mtmsr	%3;"
		:
		: "b" (vaddr), "b" (locked), "b" (tmp), "b" (msr)
	);
#else 
	ppc_tlbie(vaddr);
	ppc_eieio();
#endif	
}


void
fam_pte_flush_all() {
#if 0
	int					i;

	for(i = 0; i < TLB_INVALID_END; i += TLB_INVALID_INC) {
		fam_pte_flush_entry(i);
	}
#endif
}


void
fam_pte_init(int phase) {
	unsigned	sdr1;

	if(phase == 0) {
		// Don't support this right now (no 900 series chips turn it on)
		// See SMP_SYNC_TLBS() definition above.
		CRASHCHECK(__cpu_flags & PPC_CPU_SW_TLBSYNC);

		// Need hardware table walking
		CRASHCHECK(!(__cpu_flags & PPC_CPU_HW_HT));

#if 0 //YYY: How to tell if data cache is enabled?
		if(get_spr(PPC603_SPR_HID0) & PPC603_SPR_HID0_DCE) {
			zp_flags &= ~ZP_CACHE_OFF;
		}
#endif		

		sdr1 = get_spr(PPC_SPR_SDR1);
		HashTable_base = (void *)(sdr1 & ~((1 << 18) - 1));
		HashTable_mask = ((1 << (sdr1 & 0x1f)) - 1) << 18;

		pgszlist[0] = __PAGESIZE;
	}
	fam_pte_flush_all();
	ppc_isync();
}


/*
 * Adds an entry into the hash table 
 *
 * One has to admire the complexity of this scheme :-)
 *
 * This is only called to put in message transfer mappings. Thus, 
 * we only put in mappings in a primary slot (do not search the secondary
 * slots), and if we don't find a free slot, stick the mapping in the 
 * first primary slot. The hardware TLB refill will find it faster, 
 * and we will also find it faster when removing the mapping.
 */

static void
add_900ht_entry(uint32_t vaddr, uint32_t vsid, uint64_t lo) {
	unsigned			hash;
	struct htentry 		*hte;
	unsigned			hi;

	// Primary hash
	vsid = VSID_MAKE_REAL(vsid, vaddr);
	// Top 256M has a unique vsid for each CPU so that we can have different
	// cpupages
	if(vaddr >= 0xf0000000) vsid += RUNCPU;
	hash = PPC64_PRIMARY_HASH(vsid, vaddr);
	hte = PPC64_CALC_PTEG(hash);

	hi = (PPC64_HENTRY_AVPN(vsid, vaddr) << PPC64_TLBHI_AVPN_SHIFT) | PPC64_TLBHI_V;

	INTR_LOCK(&ht_slock);

	// Can't overwrite a permanent mapping entry from startup
	while(hte->hthi & (1 << PPC64_TLBHI_SW_SHIFT)) {
		++hte;
	}
	hte->hthi = 0x00;
	ppc_ptesync();
	hte->htlo = lo;
	ppc_eieio();
	hte->hthi = hi;
	ppc_ptesync();
	
	INTR_UNLOCK(&ht_slock);

}

/*
 * Rip out mapping for vsid from hash table and from tlb
 *
 * vaddr: virtual address to unmap
 * vsid: virtual segment ID of mapping
 */

static int
flush_900ht_entry(uintptr_t vaddr, unsigned base_vsid) {
	unsigned			hash, hi;
	struct htentry 		*shte, *phte;
	int 				i;
	int					found = 0;
	unsigned			idx;
	unsigned			vsid;

	// If mapping is in top 256M, have to clean out all the unique per-CPU
	// mappings (for cpupage).
	idx = 0;
	do {
		vsid = VSID_MAKE_REAL(base_vsid, vaddr) + idx;
		hash = PPC64_PRIMARY_HASH(vsid, vaddr);
		phte = PPC64_CALC_PTEG(hash);
		shte = (struct htentry *) ((uint32_t) 0xffffff80 & (~(HashTable_mask | 0x3ffff) ^ ~(uintptr_t) phte)); 
				
		hi = (PPC64_HENTRY_AVPN(vsid, vaddr) << PPC64_TLBHI_AVPN_SHIFT) | PPC64_TLBHI_V;

		for(i = 0; i < 8; i++, phte++, shte++){
			if(phte->hthi == hi) {
				found++;
				phte->hthi = 0x00;
			}
			if(shte->hthi == (hi | PPC64_TLBHI_H)) {
				found++;
				shte->hthi = 0x00;
			}
		}
	} while((vaddr >= 0xf0000000) && (++idx < NUM_PROCESSORS));
	ppc_ptesync();

	// Have to arbitrate tlbsync's so that CPU's don't livelock
	fam_pte_flush_entry(vaddr);	
	ppc_ptesync();
	return found;
}


/*
 * This routine is called from cpu_vmm_fault. It
 * is used to map in a page in the hash table.
 */

void
fam_pte_mapping_add(uintptr_t vaddr, paddr_t paddr, unsigned prot, unsigned flags) {
	uint64_t		lo;
	unsigned		base_vsid;

	if(!KerextAmInKernel()) {
		crash();
	}

	// Flags contains the faulting VSID in the lower 24 bits
	base_vsid = flags & 0xffffff;

	lo = paddr & ~(paddr_t)(__PAGESIZE-1);
	
	if(prot & PROT_WRITE) {
		lo |= PPC_TLBLO_PP_RW;
	} else if(prot & (PROT_READ|PROT_EXEC)) {
		lo |= PPC_TLBLO_PP_RO;
	}
	
	if(prot & PROT_NOCACHE) {
		lo |= PPC_TLBLO_G | PPC_TLBLO_I;
	}
	
	lo |= PPC_TLBLO_C | PPC_TLBLO_R | PPC_TLBLO_M;
	
	if(flush_900ht_entry(vaddr, base_vsid)) {
		SMP_SYNC_TLBS();
	}
	add_900ht_entry(vaddr, base_vsid, lo); 
}


/* 
 * Rip out mappings for vsid, from vaddr to vaddr + size
 */

void
fam_pte_mapping_del(ADDRESS *adp, uintptr_t vaddr, unsigned size) {
	unsigned	vsid = adp->cpu.asid;

	if((ADDR_OFFSET(vaddr) != 0) || (ADDR_OFFSET(size) != 0)) {
		crash();
	}
	if(vaddr == 0) {
		// Call from mem_virtual for debugging
		for(vaddr = 0x40000000; vaddr < 0xffff0000; vaddr += __PAGESIZE) {
			(void) flush_900ht_entry(vaddr, vsid);
		}
	} else {
		while(size > 0) {
			(void) flush_900ht_entry(vaddr, vsid);
			vaddr += __PAGESIZE;
			size -= __PAGESIZE;
		}
	}
	SMP_SYNC_TLBS();
}

/*
* fam_pte_asid_alloc:
*
* More than enough asid for 900's, no stealing will happen.
* Interrupt safe.
*
*/

void
fam_pte_asid_alloc(ADDRESS *adp) {
	int	idx;

	idx = vector_add(&asid_vector, adp, 0);
	if(idx != -1) {
		adp->cpu.asid = VSID_SCRAMBLE(idx);
	}
}


void
fam_pte_asid_release(ADDRESS *adp) {
	vector_rem(&asid_vector, VSID_UNSCRAMBLE(adp->cpu.asid));
}


int
cpu_pte_manipulate(struct mm_pte_manipulate *data) {
	pte_t					**pdep;
	pte_t					*pdep_l2base;
	pte_t					*ptep;
	pte_t					pte;
	pte_t					orig_pte;
	uintptr_t				l2_vaddr;
	unsigned				bits;
	pte_t					**l1pagetable;
	uint32_t				vsid;
	int						smp_flush = 0;
	struct pa_quantum		*pq;
	unsigned				pa_status;
	ADDRESS					*adp;
	unsigned				op;
	int						r;
	part_id_t				mpid;
	PROCESS *				prp;

	// Assume alignment has been checked by the caller

	adp = data->adp;
	if(adp == NULL) crash();
	l1pagetable = adp->cpu.pgdir;
	vsid = adp->cpu.asid;

	op = data->op;

	if(data->op & PTE_OP_BAD) {
		bits = PPC_TLBLO_I;
	} else {
		//RUSH3: if PTE_OP_TEMP, mark PTE as accessable from procnto only if possible
		bits = PPC_TLBLO_C | PPC_TLBLO_R;
		if(data->shmem_flags & SHMCTL_HAS_SPECIAL) {
			if(data->special & ~PPC_SPECIAL_MASK) {
				return EINVAL;
			}
			//RUSH1: If PPC_SPECIAL_E is on, should I report an error?
			// Upshift by two because PPC_SPECIAL_G is in bit position 30, but
			// The PPC_TLBLO_G bit is position 28.
			if(data->prot & (PROT_READ|PROT_WRITE|PROT_EXEC)) {
				bits |= (data->special & ~PPC_SPECIAL_E) << 2;
			}
		} else {
			bits |= PPC_TLBLO_M;
		}
		if(!(data->prot & PROT_EXEC)) {
			bits |= PPC64_TLBLO_N;
		}
		if(data->prot & PROT_WRITE) {
			 bits |= PPC_TLBLO_PP_RW;
		} else if(data->prot & (PROT_READ|PROT_EXEC)) {
			 bits |= PPC_TLBLO_PP_RO;
		} else if(op & (PTE_OP_MAP|PTE_OP_PROT)) {
			// If no permissions, treat same as unmap operation
			op |= PTE_OP_UNMAP;
		}
		if(data->prot & PROT_NOCACHE) {
			bits |= PPC_TLBLO_I;
			if(!(data->prot & PROT_EXEC)) {
				// Nocache executable memory cannot be guarded 
				bits |= PPC_TLBLO_G;
			}
		}
	}

	r = EOK;
	prp = adp ? object_from_data(adp, address_cookie) : NULL;
	mpid = mempart_getid(prp, sys_memclass_id);
	for( ;; ) {
		if(data->start >= data->end) break;
		pdep = &l1pagetable[L1PAGEIDX(data->start)];
		if(*pdep == NULL) {
			memsize_t resv = 0;

			if(!(op & (PTE_OP_MAP|PTE_OP_PREALLOC|PTE_OP_BAD))) {
				//Move vaddr to next page directory
				data->start = (data->start + PDE_SIZE) & ~(PDE_SIZE - 1);
				if(data->start == 0) data->start = ~0;
				continue;
			}
			if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
				return ENOMEM;
			}
			pq = pa_alloc(__PAGESIZE, __PAGESIZE, 0, 0, &pa_status, restrict_proc, resv);
			if(pq == NULL) {
				MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
				return ENOMEM;
			}
			MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
			pq->flags |= PAQ_FLAG_SYSTEM;
			pq->u.inuse.next = adp->cpu.l2_list;
			adp->cpu.l2_list = pq;
			l2_vaddr = pa_quantum_to_paddr(pq);
			if(pa_status & PAA_STATUS_NOT_ZEROED) {
				zero_page((uint32_t *)l2_vaddr, __PAGESIZE, NULL);
			}
			*pdep = (pte_t *)l2_vaddr;
		}
		if(op & PTE_OP_PREALLOC) {
			//Move vaddr to next page directory
			data->start = (data->start + PDE_SIZE) & ~(PDE_SIZE - 1);
			if(data->start == 0) data->start = ~0;
			continue;
		}
		pdep_l2base = *pdep;
		ptep = &(pdep_l2base[L2PAGEIDX(data->start)]);
		orig_pte = *ptep;
		if(op & PTE_OP_UNMAP) {
			pte = 0;
		} else if(op & (PTE_OP_MAP|PTE_OP_BAD)) {
			pte = data->paddr | bits;
		} else if(orig_pte & (0xfff & ~PPC_TLBLO_I)) {
			// PTE_OP_PROT
			pte = (orig_pte & ~(paddr_t)(__PAGESIZE-1)) | bits;
		}  else {
			// We don't change PTE permissions if we haven't mapped the
			// page yet...
			pte = orig_pte;
		}
		*ptep = pte;
		if((orig_pte != 0) && (pte != orig_pte)) {
			smp_flush = 1;
			(void) flush_900ht_entry(data->start, vsid);
		}

		data->start += __PAGESIZE;
		data->paddr += __PAGESIZE;
		if((op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			r = EINTR;
			break;
		}
	}
	if(smp_flush){
		SMP_SYNC_TLBS();
	}
	return r;
}

__SRCVERSION("fam_pte.c $Rev: 201493 $");
