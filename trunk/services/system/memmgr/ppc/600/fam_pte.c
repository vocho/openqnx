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

extern void add_tlb603(uint32_t hi, uint32_t lo, unsigned flag, uintptr_t addr);

/*
 * extern declarations for variables that are in the kernel. Really
 * annoying, they should go away when we merge all VM code in a single
 * entity.
 */
extern uint32_t *cpupageptr[];

static VECTOR	asid_vector;
static unsigned	num_bats;

#define BAT_EXECUTABLE	_ONEBIT32B(15)

//Temp hack until the <sys/mman.h> file is updated everywhere: 2006/12/01
#ifndef SHMCTL_HIGHUSAGE
#define SHMCTL_HIGHUSAGE 0x200
#endif

#define BAT_SIZE_MIN		(128*0x400)
#define BAT_SIZE_MAX		(256*0x400*0x400)

/*
 * Hash table entry
 */

struct htentry {
	uint32_t hthi;
	uint32_t htlo;
};

/*
 * This is the hash table base and mask 
 */

extern struct htentry 		*HashTable_base;
extern uint32_t 			HashTable_mask;
extern intrspin_t			ht_slock;

/*
 * Note: these masks much match the ones in vm700.s
 */
#define	PPC700_VSID_MASK 				0xfffff
#define PPC700_VSID(asid)				(asid & PPC700_VSID_MASK)
#define PPC700_VSID_UP(addr)			((addr & 0xf0000000) >> 1)
#define PPC700_HENTRY_VALID				0x80000000
#define PPC700_HENTRY_SEC				0x00000040
#define PPC700_HENTRY_API(vaddr)		((vaddr) & (0x0000003f))

#define PPC700_VSID_SCRAMBLE(asid)		(((asid & 0xff) << 8) | ((asid & 0xff00) >> 8) | (asid & 0xffff0000))
#define PPC700_VSID_UNSCRAMBLE(asid)		(((asid & 0xff00) >> 8) | ((asid & 0xff) << 8) | (asid & 0xffff0000))

#define SMP_SYNC_TLBS() do{\
		if(__cpu_flags & PPC_CPU_SW_TLBSYNC){\
			SMP_FLUSH_TLB();\
		}\
	}while(0)

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
 

static int
fam_pte_flush_entry(unsigned vaddr) {
	intrspin_t 		*locked = &ht_slock;
	uint32_t		tmp = 0, msr = 0;

	asm volatile(
#ifdef VARIANT_smp
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
#endif
		"	tlbie	%0;"
		"	eieio;"
#ifdef VARIANT_smp
		"	tlbsync;"
		"	sync;"
		"	li		%2,0;"
		"	stw 	%2,0(%1);"
		"	sync;"
		"	mtmsr	%3;"
#endif
		:
		: "b" (vaddr), "b" (locked), "b" (tmp), "b" (msr)
	);

	return 1;
}


void
fam_pte_flush_all() {
	int					i;

	for(i = 0; i < TLB_INVALID_END; i += TLB_INVALID_INC) {
		(void) fam_pte_flush_entry(i);
	}
}


static int
calc_total_ram(struct asinfo_entry *as, char *name, void *data) {
	uint64_t	*total_ram = data;

	*total_ram += ( as->end - as->start ) + 1;
	return 1;
}

static void
alloc_hashtable() {
	struct pa_quantum	*qp;
	unsigned 			ram;
	unsigned			size;
	uint64_t			total_ram;
	memsize_t			resv = 0;
	// FIX ME - this allocation accounted to system
	part_id_t			mpid = mempart_getid(NULL, sys_memclass_id);

	total_ram = 0;
	walk_asinfo("ram", calc_total_ram, &total_ram);

	// Calculate the size of the hash table needed; round up
	// -> ram = total + total - 1
	// The "| 0x10000" is because 0x10000 is the smallest legal size 
	// for the hash table. By forcing that bit on we ensure that 
	// the while loop below terminates at the minimum allowed size.
	ram = ((total_ram >> 6) - 1) | 0x10000;
	// find highest bit set
	size = 0x80000000;
	while((size & ram) == 0) {
		size >>= 1;
	} 
	
	for( ;; ) {
		// Now alloc memory for page table. We need memory aligned
		// on a 'size' boundary.
		if(MEMPART_CHK_and_INCR(mpid, size, &resv) == EOK) {
			qp = pa_alloc(size, size, 0, PAA_FLAG_CONTIG, NULL, restrict_proc, resv);
			if(qp != NULL) break;
			// Couldn't get the recomended size, so we'll go for
			// something a little smaller and live with slower performance.
			MEMPART_UNDO_INCR(mpid, size, resv);
		}
		if(size == 0x10000) {
			kprintf("Could not allocate hash table\n");
			crash();
		}
		size >>= 1;
	}
	
	// The mask is that used to generate the hash function.
	// HTABMASK = HashTable_mask >> 11
	HashTable_mask = (size - 1) & 0x01ff0000;
	MEMCLASS_PID_USE(NULL, mempart_get_classid(mpid), size);
	qp->flags |= PAQ_FLAG_SYSTEM;
	HashTable_base = (void *)(uintptr_t)pa_quantum_to_paddr(qp);
	memset(HashTable_base, 0, size);
}


void
fam_pte_init(int phase) {
	int			idx0;
	int			idx1;
	uintptr_t	sz;

	if(phase == 0) {
		idx0 = 0;
		for(sz = BAT_SIZE_MAX; sz >= BAT_SIZE_MIN; sz >>= 1) {
			pgszlist[idx0++] = sz;
		}
		pgszlist[idx0] = __PAGESIZE;

		// Reserve ASIDs for kernel and proc.
		//RUSH3: Do we need both (or either) of these reservations now?
		idx0 = vector_add(&asid_vector, (void *)4, 0);
		idx1 = vector_add(&asid_vector, (void *)8, 0);
	
		if((idx0 != 0) || (idx1 != 1)) {
			crash();
		}

		if(get_spr(PPC603_SPR_HID0) & PPC603_SPR_HID0_DCE) {
			zp_flags &= ~ZP_CACHE_OFF;
		}
		if(__cpu_flags & PPC_CPU_HW_HT) {
			alloc_hashtable();
		}
		num_bats = (__cpu_flags & PPC_CPU_EXTRA_BAT) ? 8 : 4;
	}

	/*
	 * Bat Registers are set up by startup so that {I,D}BAT0 does a
	 * a one-to-one mapping of the first 256M of memory. The remainder
	 * have mappings for device memory required by the callouts.
	 * If we're SMP and DBAT3 is already in use, we're in trouble.
	 */
		
#ifdef VARIANT_smp

	if(NUM_PROCESSORS > 1) {
		unsigned	i;
		unsigned	adj;
		uintptr_t	cpupage = (uintptr_t)cpupageptr[RUNCPU];

		if(cpupage & (0x20000 - 1)) {
			kprintf("CPU page not 128kb aligned\n");
			crash();
		}
	
		i = 0;
		for(;;) {
			if(i >= num_bats) {
				kprintf("No BAT to map cpupage - too many callout mappings\n");
				crash();
			}
			adj = i*2;
			if(i >= 4) {
				// massage the adjustment for the 4 high order bats
				adj += (PPC_SPR_IBAT4U - PPC_SPR_IBAT0U) - 8;
			}
			if(!(get_spr_indirect(PPC_SPR_DBAT0U + adj) & (PPC_BATU_VS|PPC_BATU_VP))) break;
			++i;
		}
					
		set_spr_indirect(PPC_SPR_IBAT0L+adj, PPC_BATL_BRPN(cpupage >> 17) | PPC_BATL_PP_R | PPC_BATL_M);
		set_spr_indirect(PPC_SPR_IBAT0U+adj, PPC_BATU_BEPI(VM_CPUPAGE_ADDR >> 17) | PPC_BATU_BL_128K | PPC_BATU_VS | PPC_BATU_VP);
		set_spr_indirect(PPC_SPR_DBAT0L+adj, PPC_BATL_BRPN(cpupage >> 17) | PPC_BATL_PP_R | PPC_BATL_M);
		set_spr_indirect(PPC_SPR_DBAT0U+adj, PPC_BATU_BEPI(VM_CPUPAGE_ADDR >> 17) | PPC_BATU_BL_128K | PPC_BATU_VS | PPC_BATU_VP);

		if(__cpu_flags & PPC_CPU_DCBZ_NONCOHERENT) {
			// Can't use DCBZ if there's more than one processor, since
			// a snoop at the same time as the DCBZ can cause stale
			// data.
			zp_flags |= ZP_DCBZ_BAD;
		}
	}

#endif

	if(__cpu_flags & PPC_CPU_HW_HT) {
		set_spr(PPC_SPR_SDR1, (uintptr_t) HashTable_base | (HashTable_mask >> 16));
		ppc_isync();
		ppc_sync();
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
add_700ht_entry(uint32_t hi, uint32_t lo, uint32_t vaddr, uint32_t vsid) {
	uint32_t			hash;
	uint32_t			base = (uint32_t) HashTable_base;
	struct htentry 		*hte;


	// Primary hash
	hash = (vsid & 0x7ffff) ^ ((vaddr >> 12) & 0xffff);
	hte = (struct htentry *) ((base & 0xf7000000) |
		((base & 0x08ff0000) | 
			(HashTable_mask & ((hash & 0x7fc00) << 6))) |
				((hash & 0x3ff) << 6));

	// Since this is only called for msg transfer, take the first
	// slot, so we will also find it quickly when unmapping.

	INTR_LOCK(&ht_slock);

	hte->hthi = 0x00;
	ppc_sync();
	hte->htlo = lo;
	asm volatile( "eieio");
	hte->hthi = hi;
	ppc_sync();
	
	INTR_UNLOCK(&ht_slock);

}

/*
 * Rip out mapping for vsid from hash table and from tlb
 *
 * vaddr: virtual address to unmap
 * vsid: virtual segment ID (24 bits) of mapping
 */

static int
flush_700ht_entry(uint32_t vaddr, uint32_t vsid) {

	uint32_t			hash, hi;
	uint32_t			base = (uint32_t) HashTable_base;
	struct htentry 		*hte, *phte;
	int 				i;
	int					found = 0;


	if(__cpu_flags & PPC_CPU_HW_HT) {

		hi = 	(PPC700_HENTRY_API(vaddr>>22)) 
				| _BITFIELD32B(24, vsid) 
				| PPC700_VSID_UP(vaddr)
				| PPC700_HENTRY_VALID;
		hash = (vsid & 0x7ffff) ^ ((vaddr >> 12) & 0xffff);
		phte = hte = 	(struct htentry *) ((base & 0xf7000000) |
					((base & 0x08ff0000) | 
				(HashTable_mask & ((hash & 0x7fc00) << 6))) |
				((hash & 0x3ff) << 6));
				
		for(i = 0;i < 8; i++, hte++){
			if(hte->hthi == hi) {
				found++;
				hte->hthi = 0x00;
			}
		}

		hte = (struct htentry *) ((uint32_t) 0xffffffc0 & (~(HashTable_mask | 0xffff) ^ ~(uint32_t) phte)); 

		hi |= PPC700_HENTRY_SEC;
		for(i = 0;i < 8; i++, hte++){
			if(hte->hthi == hi) {
				found ++;
				hte->hthi = 0x00;
			}
		}
	}

	// Have to arbitrate tlbsync's so that CPU's don't livelock
	(void) fam_pte_flush_entry(vaddr);	
	return found;
}


/*
 * This routine is called from cpu_vmm_fault. It
 * is used to map in a page in the hash table.
 */

void
fam_pte_mapping_add(uintptr_t vaddr, paddr_t paddr, unsigned prot, unsigned flags) {
	unsigned		lo;
	unsigned		hi;

	if(!KerextAmInKernel()) {
		crash();
	}

	// Flags contains the faulting VSID in the lower 24 bits
	hi = 	(PPC700_HENTRY_API(vaddr>>22)) 
			| _BITFIELD32B(24, PPC700_VSID(flags)) 
			| PPC700_VSID_UP(vaddr)
			| PPC700_HENTRY_VALID;

	lo = paddr & _BITFIELD32B(19, 0xfffff);
	
	if(prot & PROT_WRITE) {
		lo |= PPC_TLBLO_PP_RW;
	} else if(prot & (PROT_READ|PROT_EXEC)) {
		lo |= PPC_TLBLO_PP_RO;
	}
	
	if(prot & PROT_NOCACHE) {
		lo |= PPC_TLBLO_G | PPC_TLBLO_I;
	}
	
	lo |= PPC_TLBLO_C | PPC_TLBLO_R | PPC_TLBLO_M;
	
   	//NYI: should respect VM_FAULT_GLOBAL flag (see book E implementation)
	if(__cpu_flags & PPC_CPU_HW_HT) {
		if(flush_700ht_entry(vaddr, PPC700_VSID(flags))) {
			SMP_SYNC_TLBS();
		}
		add_700ht_entry(hi, lo, vaddr, PPC700_VSID(flags)); 
	} else {
		// Must be a 603
		add_tlb603(hi, lo, flags & VM_FAULT_INSTR, vaddr);
	}
}


/* 
 * Rip out mappings for vsid, from vaddr to vaddr + size
 */

void
fam_pte_mapping_del(ADDRESS *adp, uintptr_t vaddr, unsigned size) {
	unsigned	vsid = PPC700_VSID(adp->cpu.asid);

	if((ADDR_OFFSET(vaddr) != 0) || (ADDR_OFFSET(size) != 0)) {
		crash();
	}
	if(vaddr == 0) {
		// Call from mem_virtual for debugging
		for(vaddr = 0x40000000; vaddr < 0xffff0000; vaddr += __PAGESIZE) {
			flush_700ht_entry(vaddr, vsid);
		}
	} else {
		while(size > 0) {
			flush_700ht_entry(vaddr, vsid);
			vaddr += __PAGESIZE;
			size -= __PAGESIZE;
		}
	}
	SMP_SYNC_TLBS();
}

/*
* fam_pte_asid_alloc:
*
* More than enough asid for ppc 600 and 700, no stealing will happen.
* Interrupt safe.
*
*/

void
fam_pte_asid_alloc(ADDRESS *adp) {
	int	idx;

	idx = vector_add(&asid_vector, adp, 0);
	if(idx != -1) {
		adp->cpu.asid = PPC700_VSID_SCRAMBLE(idx);
	}
	// mark all the permanently allocated entries.
	if(adp == procnto_prp->memory) {
		unsigned	i;

		for(i = 0; i < num_bats; ++i) {
			unsigned	bat;

			if(i >= 4) {
				bat = PPC_SPR_DBAT4U + (i - 4) * 2;
			} else {
				bat = PPC_SPR_DBAT0U + i * 2;
			}
			if(get_spr_indirect(bat) & (PPC_BATU_VS|PPC_BATU_VP)) {
				// Mark the entry as permanenty allocated
				procnto_prp->memory->cpu.bat[i].up = ~0;
			}
		}
	} else {
		memcpy(adp->cpu.bat, procnto_prp->memory->cpu.bat, sizeof(adp->cpu.bat));
	}
	// Mark user segment regions as having no execute permissions.
	// There's code on the system page, so we have to mark that
	// region as executable.
	adp->cpu.nx_state = 
		(((1 << ((CPU_USER_VADDR_END >> 28)+1)) - 1)
		& ~((1 << ((CPU_USER_VADDR_START >> 28))) - 1))
		& ~(1 << (VM_SYSPAGE_ADDR >> 28));
}	


void
fam_pte_asid_release(ADDRESS *adp) {
	vector_rem(&asid_vector, PPC700_VSID_UNSCRAMBLE(adp->cpu.asid));
}


static void
set_one_bat(ADDRESS *adp, unsigned bi) {
	unsigned				adj;
	unsigned				lo;
	unsigned				up;

	adj = bi*2;
	if(bi >= 4) {
		// massage the adjustment for the 4 high order bats
		adj += ( PPC_SPR_IBAT4U - PPC_SPR_IBAT0U ) - 8;
	}
	up = adp->cpu.bat[bi].up;
	if(up != ~0U) {
		// not a permanently set entry
		lo = adp->cpu.bat[bi].lo;
		set_spr_indirect(PPC_SPR_DBAT0L + adj, lo);
		set_spr_indirect(PPC_SPR_DBAT0U + adj, up & ~BAT_EXECUTABLE);
		if(!(up & BAT_EXECUTABLE)) {
				lo = 0;
				up = 0;
		}
		set_spr_indirect(PPC_SPR_IBAT0L + adj, lo & ~(PPC_BATL_G|PPC_BATL_W));
		set_spr_indirect(PPC_SPR_IBAT0U + adj, up & ~BAT_EXECUTABLE);
	}
}


// Override the standard PPC aspace switch code with one that save/restores
// the bats - only gets installed if someone actually uses them.
static void
vmm_aspace_bat(PROCESS *actprp, PROCESS **pactprp) {
	ADDRESS					*adp;
	unsigned				i;

	if((adp = actprp->memory)) {
		InterruptDisable();
		set_l1pagetable(adp->cpu.pgdir, adp->cpu.asid, adp->cpu.nx_state);
		for(i = 0; i < num_bats; ++i) {
			set_one_bat(adp, i);
		}
		*pactprp = actprp;
		InterruptEnable();
	}
}



#define TLB_PADDR_BITS(a)	(a & 0xfffffe04)
#define PADDR_TO_TLB(p)		((p & 0xfffff000) | ((p>>30) & 0x04) | ((p>>24)&0x00000e00))


#define GET_ACTIVE_ASID()	((get_sreg(0x40000000U) & 0x00ffffffU) - 0x400000U)

static void
clean_bats(struct mm_pte_manipulate *data, uintptr_t start, uintptr_t end) {
	unsigned	i;
	unsigned	up;
	uintptr_t	bat_start;
	uintptr_t	bat_end;
	ADDRESS 	*adp; 

	adp = data->adp;
	// Check for BATs in the range
	for(i = 0; i < num_bats; ++i) {
		up = adp->cpu.bat[i].up;
		switch(up) {
		case ~0U:	// permanent entry
		case 0:		// unused entry
			break;
		default:	
			bat_start = up & PPC_BATU_BEPI_MASK;
			bat_end = bat_start 
				+ ((((up & PPC_BATU_BL_MASK)|0x3)+1) << (17-2)) - 1;
			if(!((bat_end < start) || (bat_start > end))) {
				// bat is in the range

				// Make sure the indicator bit is on so that the 
				// cpu_pte_merge code knows to try to re-establish
				// the BAT (would be off for PTE_OP_UNMAP)
				data->shmem_flags |= SHMCTL_HIGHUSAGE;
				if(bat_end > data->split_end) {
					data->split_end = bat_end;
				}
				if(GET_ACTIVE_ASID() == adp->cpu.asid) {
					// We're manipulating the active address space,
					// have to clear the BAT.
					adp->cpu.bat[i].up &= ~(PPC_BATU_VS|PPC_BATU_VP);
					set_one_bat(adp, i);
				}
				adp->cpu.bat[i].up = 0;
			}
			break;
		}
	}
}


int
cpu_pte_split(uintptr_t vaddr, struct mm_pte_manipulate *data) {
	if(vaddr != data->start) {
		// Took care of everything with the first call
		return EOK;
	}
	if((data->op & PTE_OP_UNMAP) || (data->shmem_flags & SHMCTL_HIGHUSAGE)) {
		clean_bats(data, data->start, data->end);
	}
	return EOK;
}


int
cpu_pte_merge(struct mm_pte_manipulate *data) {
	pte_t			**pgdir;
	pte_t			*pde;
	pte_t			*ptep;
	pte_t			*chk_ptep;
	unsigned		pgsz;
	uintptr_t		run_start;
	uintptr_t		run_last;
	uintptr_t		chk_vaddr;
	uintptr_t		merge_start;
	uintptr_t		merge_end;
	paddr_t			paddr;
	unsigned		bi;
	ADDRESS			*adp;

	if(!(data->shmem_flags & SHMCTL_HIGHUSAGE)) {
		// We only want to try to use the BAT's if SHMCTL_HIGHUSAGE is on
		return EOK;
	}

	run_start = run_last = 0; // Shut up GCC warning.

	adp = data->adp;
	pgdir = adp->cpu.pgdir;
	if(!(data->op & PTE_OP_MERGESTARTED)) {
		// look at the PTE's in front of data->start and see if they're
		// contiguous - we might be able to merge them in now. 
		chk_vaddr = data->start;
		// try merging to the start of the largest available pagesize in
		// front of the manipulated region.
		merge_start = ROUNDDOWN(chk_vaddr, BAT_SIZE_MAX);
		chk_ptep = NULL;
		for( ;; ) {
			pde = pgdir[L1PAGEIDX(chk_vaddr)];
			if(pde == NULL) break;
			ptep = &pde[L2PAGEIDX(chk_vaddr)];
			if(!PTE_PRESENT(ptep)) break;
			if(chk_ptep != NULL) {
				ptep = &pgdir[L1PAGEIDX(chk_vaddr)][L2PAGEIDX(chk_vaddr)];
				if(PTE_PERMS(ptep) != PTE_PERMS(chk_ptep)) break;
				if((PTE_PADDR(ptep) + __PAGESIZE) != PTE_PADDR(chk_ptep)) break;
				data->start = chk_vaddr;
			}
			if(chk_vaddr == merge_start) break;
			chk_ptep = ptep;
			chk_vaddr -= __PAGESIZE;
		}
		data->op |= PTE_OP_MERGESTARTED;
	}
	// try merging to the end of the largest available pagesize beyond
	// the end of the manipulated region.
	merge_end = ROUNDUP(data->split_end + 1, BAT_SIZE_MAX) - 1;
	do {
		chk_ptep = NULL;
		for( ;; ) {
			if(data->start >= merge_end) break;
			pde = pgdir[L1PAGEIDX(data->start)];
			if(pde == NULL) break;
			ptep = &pde[L2PAGEIDX(data->start)];
			if(!PTE_PRESENT(ptep)) break;
			if(chk_ptep == NULL) {
				run_start = data->start;
			} else {
				if(PTE_PERMS(ptep) != PTE_PERMS(chk_ptep)) break;
				if((PTE_PADDR(chk_ptep) + __PAGESIZE) != PTE_PADDR(ptep)) break;
			}
			run_last = data->start;
			data->start += __PAGESIZE;
			if(data->start == 0) data->start = ~(uintptr_t)0;
			chk_ptep = ptep;
		}

		if(chk_ptep != NULL) {
			// We've got a run of PTE's with perms that are the same and the
			// paddrs are contiguous. Start building up larger page sizes
			// based on the vaddr/paddr alignment
			while(run_start < run_last) {
				ptep = &pgdir[L1PAGEIDX(run_start)][L2PAGEIDX(run_start)];
				paddr = PTE_PADDR(ptep);
				pgsz = BAT_SIZE_MAX << 1;
				// Find a page size that works
				do {
					pgsz >>= 1;
				} while((pgsz > ((run_last-run_start)+__PAGESIZE)) 
					  || ((run_start & (pgsz-1)) != 0)
					  || ((paddr & (pgsz-1)) != 0));

				if(pgsz >= BAT_SIZE_MIN) {
					// Find a BAT to use

					// We can't depend on cpu_pte_split() to have removed
					// all the bat's covering this range because the run
					// might be starting from before data->start.
					clean_bats(data, run_start, run_last);

					bi = 0;
					for( ;; ) {
						if(bi >= num_bats) return EOK; // no more free bats.
						if(adp->cpu.bat[bi].up == 0) break;
						++bi;
					}
					adp->cpu.bat[bi].lo = *ptep & ~(PPC_TLBLO_R|PPC_TLBLO_C);
					adp->cpu.bat[bi].up = run_start | (PPC_BATU_VS|PPC_BATU_VP)
							| ((pgsz-1) >> (17-2));
					if(PTE_EXECUTABLE(ptep)) {
						adp->cpu.bat[bi].up |= BAT_EXECUTABLE;
					}
					if(GET_ACTIVE_ASID() == adp->cpu.asid) {
						set_one_bat(adp, bi);
					}
					memmgr.aspace = vmm_aspace_bat;
//kprintf("BAT %d @ %x for size %x, up/lo=%x/%x\n", bi, run_start, pgsz, adp->cpu.bat[bi].up, adp->cpu.bat[bi].lo);
				}
				run_start += pgsz;
			}
		} else {
			data->start += __PAGESIZE;
			if(data->start == 0) data->start = ~(uintptr_t)0;
		}
		if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			return EINTR;
		}
	} while(data->start < data->split_end);
	return EOK;
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
	vsid = PPC700_VSID(adp->cpu.asid);

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
		if(!(op & PTE_OP_TEMP) && (op & (PTE_OP_MAP|PTE_OP_PROT)) && (data->prot & PROT_EXEC)) {
			// We have PROT_EXEC specified for something. Have to turn
			// off the no-execute indicator for the segment register range
			//
			// Note - for temporary mappings we don't need to knock down the bit, since they will
			// never be executed.
			adp->cpu.nx_state &= ~(1 << (data->start >> 28));
		}
		pdep = &l1pagetable[L1PAGEIDX(data->start)];
		if(*pdep == NULL) {
			memsize_t  resv = 0;

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
			pte = PADDR_TO_TLB(data->paddr) | bits;
		} else if(orig_pte & (0xfff & ~PPC_TLBLO_I)) {
			// PTE_OP_PROT
			pte = TLB_PADDR_BITS(orig_pte) | bits;
		}  else {
			// We don't change PTE permissions if we haven't mapped the
			// page yet...
			pte = orig_pte;
		}
		*ptep = pte;
		if((orig_pte != 0) && (pte != orig_pte)) {
			smp_flush = 1;
			flush_700ht_entry(data->start, vsid);
		}

		data->start += __PAGESIZE;
		data->paddr += __PAGESIZE;
		if((op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			r = EINTR;
			break;
		}
	}
	if(smp_flush) SMP_SYNC_TLBS();
	return r;
}

__SRCVERSION("fam_pte.c $Rev: 209778 $");
