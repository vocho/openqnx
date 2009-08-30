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
#include <sh/ccn.h>
#include <externs.h>

//#define CPU_VMM_DEBUG

/* permanent tlb */
struct sh4_tlb {
	uintptr_t	vaddr;
	_Uint32t	data;
};

static struct sh4_tlb                   *permanent_tlb_entries;
static unsigned                         num_permanent_tlb;

#define PTE_ATTR(pte)			((pte) & (__PAGESIZE-1) )
#define PTE_PERMS(pte)			(PTE_ATTR(pte) & ~(SH_CCN_PTEL_SZ0 | SH_CCN_PTEL_SZ1) )

#define PTE_PGSIZE_TO_ATTR(size)	((size == 4096)?SH_CCN_PTEL_SZ0:(size == MEG(1))?(SH_CCN_PTEL_SZ0 |SH_CCN_PTEL_SZ1):SH_CCN_PTEL_SZ1)


void
cpu_pte_init(void) {
	pgszlist[0] = MEG(1);
	pgszlist[1] = KILO(64);
	pgszlist[2] = KILO(4);
}



void
sh4_update_tlb(uintptr_t vaddr, uint32_t ptel) {
	unsigned intr, pteh;

	intr = SH_SR_IMASK & get_sr();

	if(intr != SH_SR_IMASK) {
		InterruptDisable();
	}

	//The pteh is created from the existing PTEH.asid and the vaddr page address
	pteh = in32(SH_MMR_CCN_PTEH);
	pteh = (vaddr & SH_PTE_PGSIZE_MASK(ptel)) | (pteh & SH_CCN_PTEH_ASID_M);
	out32(SH_MMR_CCN_PTEH, pteh);

	//The ptel is given to us
	out32(SH_MMR_CCN_PTEL, ptel);

	//ldtlb
	load_tlb();

	if(intr != SH_SR_IMASK) {
		InterruptEnable();
	}
}




/*
 * perm_map_check: check for a fault on one of the permanent entries and
 *      (optionally) put it back in the TLB if need be.
 */
static struct sh4_tlb *
perm_map_check(uintptr_t badva, unsigned write_fault, unsigned load_tlb) {
	int                        i;
	uintptr_t                  mask;
	struct sh4_tlb             *perm;

	if(!num_permanent_tlb) return NULL;

	/* Check for one of the permanent mappings */
	for(i = num_permanent_tlb-1; i >= 0; --i) {
		perm = &permanent_tlb_entries[i];
		mask = SH_PTE_PGSIZE_MASK(perm->data);
		if((perm->vaddr & mask) == (badva & mask)) {

			/* If a write fault and perm_mapping is R/O, early out */
			/* @@@ assumes entrylo0 and entrylo1 have same prot */
			if(write_fault && !(perm->data & SH_CCN_PTEL_PR(1))) return 0;

			/* Found a permanent entry! Slap it in and get out */
			if(load_tlb) {
				sh4_update_tlb(perm->vaddr, perm->data);
			}
			#ifdef CPU_VMM_DEBUG
			kprintf("check permanent map:vaddr %x data %x load %d\n",perm->vaddr,perm->data,load_tlb);
			#endif
			return perm;
		}
	}
	return NULL;
}

static void
sh4_gettlb(struct sh4_tlb *tlb, unsigned i) {
	unsigned 	ptr;
	uint32_t	value;

	i <<= 8;
	ptr = i + SH4_UTLB_ADDRESS_ARRAY;
	tlb->data = 0;
	value = sh_tlb_in32(ptr) ;
	if(value & SH_CCN_PTEL_V) {
		tlb->vaddr = REAL_ADDR(value);
		ptr = i + SH4_UTLB_DATA_ARRAY1;
		value = sh_tlb_in32(ptr) ;
		if(value & SH_CCN_PTEL_V) {
			tlb->data = value;
		}
	}
}


void
perm_map_init(void) {
	unsigned                                i;
	unsigned                                j;
	struct sh4_tlb                  *perm;
	struct sh4_tlb                  tlb;

	/*
	 * Collect all the permanent TLB mappings.
	 */

	j = 0;
	for(i = 0; i < SH4_UTLB_SIZE; ++i) {
		sh4_gettlb(&tlb, i);
		if(tlb.data & SH_CCN_PTEL_V) {
			++j;
		}
	}
	num_permanent_tlb = j;

	if(j != 0) {
		perm = malloc(sizeof(*permanent_tlb_entries)*num_permanent_tlb);
		if(perm == NULL) {
			crash();
		}
		permanent_tlb_entries = perm;
		for(i = 0; i < SH4_UTLB_SIZE; ++i) {
			sh4_gettlb(&tlb, i);
			if(tlb.data & SH_CCN_PTEL_V) {
				if(tlb.vaddr < PERMANENT_MAP_START || tlb.vaddr > PERMANENT_MAP_END) {
					num_permanent_tlb--;
					kprintf("Wrong permanent mapping passed from startup: vaddr %x!\n",tlb.vaddr);
					continue;
				}
				*perm = tlb;

				#ifdef CPU_VMM_DEBUG
				kprintf("Found one permanent map:tlb vaddr %x data %x\n",perm->vaddr,perm->data);
				#endif
				++perm;
			}
		}
	}
	#ifdef CPU_VMM_DEBUG
	kprintf("num perm tlb:%d\n",j);
	#endif
}

/*
 * tlb_flush_all()
 *       Flush all TLB entries
 */
void
tlb_flush_all(void) {
	out32(SH_MMR_CCN_MMUCR, in32(SH_MMR_CCN_MMUCR) | SH_CCN_MMUCR_TI);
}

#ifdef	VARIANT_smp
void
smp_tlb_sync(PROCESS *prp)
{
	ADDRESS		*adp = prp->memory;
	unsigned	cpu = (1 << RUNCPU);

	if (adp && (adp->cpu.asid_flush & cpu)) {
		atomic_clr(&adp->cpu.asid_flush, cpu);
		tlb_flush_asid(adp->cpu.asid);
	}
}
#endif


/*
 * tlb_flush_va()
 *       Flush TLB associated with a particular virtual address range
 */
void
tlb_flush_va(struct mm_aspace *adp, uintptr_t start, uintptr_t end) {
	unsigned 	index;
	uint32_t	ptel, pteh;
	uint32_t	mask;
	uintptr_t	vaddr, ppteh;
	unsigned	intr;
	unsigned	asid;

	intr = SH_SR_IMASK & get_sr();

	if(intr != SH_SR_IMASK) {
		InterruptDisable();
	}

	asid = adp->cpu.asid;

	// flush itlb buffer
	for(index=0;index<SH4_ITLB_SIZE;index++) {
		ptel = sh_tlb_in32( (index<<8) + SH4_ITLB_DATA_ARRAY1 );
		mask = SH_PTE_PGSIZE_MASK(ptel);
		ppteh = (index<<8) + SH4_ITLB_ADDRESS_ARRAY;
		pteh = sh_tlb_in32( ppteh );
		vaddr = pteh & mask;
		// tricky logic: "if ( (the asid matches) and (the range overlaps the pte) )" ...
		// where we test the range overlap with "not [ range ends before pte or range begins
		// after pte ]"
		if( (SH_CCN_ASID_FROM_PTEH(pteh)== asid) && !( (end < vaddr) || (start > vaddr+SH_PTE_PGSIZE(ptel)-1) ) ) {
			sh_tlb_out32(ppteh,0);
		}
	}

	// flush utlb buffer
	for(index=0;index<SH4_UTLB_SIZE - NUM_TLB_RESERVED;index++) {
		ptel = sh_tlb_in32( (index<<8) + SH4_UTLB_DATA_ARRAY1 );
		mask = SH_PTE_PGSIZE_MASK(ptel);
		ppteh = (index<<8) + SH4_UTLB_ADDRESS_ARRAY;
		pteh = sh_tlb_in32( ppteh );
		vaddr = pteh & mask;
		if( (SH_CCN_ASID_FROM_PTEH(pteh)== asid) && !( (end < vaddr) || (start > vaddr+SH_PTE_PGSIZE(ptel)-1) ) ) {
			sh_tlb_out32(ppteh,0);
		}
	}

	if(intr != SH_SR_IMASK) {
		InterruptEnable();
	}
#ifdef	VARIANT_smp
{
	unsigned	runcpu = RUNCPU;
	unsigned	i;

	/*
	 * Indicate that all cpus other than RUNCPU need to flush their TLBs
	 */
	atomic_set(&adp->cpu.asid_flush, LEGAL_CPU_BITMASK & ~(1 << RUNCPU));
	for (i = 0; i < NUM_PROCESSORS; i++) {
		if (i != runcpu) {
			SENDIPI(i, IPI_TLB_FLUSH);
		}
	}
}
#endif
}


/*
 * tlb_flush_asid()
 *       Flush TLB associated with a particular ASID
 */
void
tlb_flush_asid(unsigned asid) {
	uint32_t index, ptr, value;
	unsigned intr;

	intr = SH_SR_IMASK & get_sr();

	if(intr != SH_SR_IMASK) {
		InterruptDisable();
	}

	// flush itlb buffer
	for(index = 0; index < SH4_ITLB_SIZE; index++) {
		ptr = (index<<8) + SH4_ITLB_ADDRESS_ARRAY;
		value = sh_tlb_in32(ptr) ;
		if((SH_CCN_PTEH_ASID_M & value) == asid) {
			sh_tlb_out32(ptr,0);
		}
	}

	// flush utlb buffer
	for(index = 0; index < (SH4_UTLB_SIZE - NUM_TLB_RESERVED); index++) {
		ptr = (index<<8) + SH4_UTLB_ADDRESS_ARRAY;
		value = sh_tlb_in32(ptr) ;
		if((SH_CCN_PTEH_ASID_M & value) == asid) {
			sh_tlb_out32(ptr,0);
		}
	}

	if(intr != SH_SR_IMASK) {
		InterruptEnable();
	}
}

int
cpu_vmm_fault(struct fault_info *info) {
	uintptr_t		vaddr;
	PROCESS			*prp;
	unsigned		sigcode;
	uint32_t		pte;
	uint32_t		*pde;
	ADDRESS			*adp;

	vaddr = info->vaddr;
	prp = info->prp;

	sigcode = info->sigcode;

	// check permanent mapping
	if(vaddr >= PERMANENT_MAP_START && vaddr < PERMANENT_MAP_END && SIGCODE_SIGNO(sigcode) == SIGSEGV) {
		if(perm_map_check(vaddr, FAULT_ISWRITE(sigcode), 1) != NULL) {
			return 1;
		}
	}

	adp = prp->memory;
	if(!adp) {
		return 0;
	}

	pde = adp->cpu.pgdir[L1PAGEIDX(vaddr)];
	if(pde) {
		pte = pde[L2PAGEIDX(vaddr)];
		if((pte & (SH_CCN_PTEL_V|SH_CCN_PTEL_PR(3))) == SH_CCN_PTEL_PR(3)) {
			// This page has been marked as permanently bad
			return -2;
		}
	}

	/*
	 * FIXME: may want to rationalise the address range used...
	 */
	if((vaddr >= VM_MSG_XFER_START) && (vaddr < VM_MSG_XFER_END) && (sigcode & SIGCODE_INXFER)) {
		struct xfer_map	*map = &xfer_map[RUNCPU];

		if (map->prp && (map->size0 || map->size1)) {
			if (vaddr < VM_MSG_XFER_START + map->size0) {
				info->prp = map->prp;
				info->vaddr = vaddr - map->diff0;
			}
			else if (map->size1) {
				info->prp = map->prp;
				info->vaddr = vaddr - map->diff1;
			}
		}
	}

	return 0;
}


static void *
alloc_pgentry(PROCESS *prp) {
	struct pa_quantum	*pq;
	paddr_t				paddr;
	void				*vaddr;
	unsigned			pa_status;
	memsize_t			resv = 0;
	part_id_t		mpid = mempart_getid(prp, sys_memclass_id);

	if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
		return NULL;
	}
	pq = pa_alloc(__PAGESIZE, __PAGESIZE, PAQ_COLOUR_NONE, 0, &pa_status, restrict_proc, resv);
	if(pq == NULL) {
		MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
		return NULL;
	}
	MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
	pq->flags |= PAQ_FLAG_SYSTEM;
	paddr = pa_quantum_to_paddr(pq);
	mem_page_cache_invalidate_phy(paddr, __PAGESIZE);
	PAQ_SET_COLOUR(pq, PAQ_COLOUR_NONE);
	// zeroing through cached mapping, then flush cache
	vaddr = (void *)SH_PHYS_TO_P1(paddr);
	CPU_ZERO_PAGE(vaddr, __PAGESIZE, 0);
	{
		uint32_t	linesize;
		struct cacheattr_entry *cacheattr;
		uintptr_t	ptr,end;

		cacheattr = SYSPAGE_ENTRY(cacheattr);
		cacheattr += SYSPAGE_ENTRY(cpuinfo)->data_cache;
		linesize = cacheattr->line_size;
		for(ptr = (uintptr_t)vaddr, end = ptr + __PAGESIZE; ptr < end; ptr += linesize) {
			dcache_flush(ptr);
		}
	}

	vaddr = (void *)SH_PHYS_TO_P2(paddr);
	return vaddr;
}


int
cpu_vmm_mcreate(PROCESS *prp) {
	ADDRESS						*adp;
	void						*vaddr;
	int							r;
	paddr_t						paddr;

	r = ENOMEM;

	/*
	 * allocate and zero the level1 page table.
	 */
	vaddr = alloc_pgentry(prp);
	if(vaddr == NULL) goto fail1;

	adp = prp->memory;
	adp->cpu.pgdir = vaddr;
	adp->cpu.asid = ~0;

	// For ClockCycles()
	if ( __cpu_flags & SH_CPU_FLAG_TMU_THROUGH_A7 ) {
		paddr = P4_TO_A7(sh_mmr_tmu_base_address); // Need to look through the A7 area
	} else {
		paddr = sh_mmr_tmu_base_address;
	}
	r = pte_map(adp, VM_TIMERPAGE_ADDR, VM_TIMERPAGE_ADDR+__PAGESIZE-1,
	            PROT_READ|PROT_NOCACHE, NULL, paddr, 0);
	if(r != EOK) goto fail2;

	// For backwards compatability
	paddr = P4_TO_A7(SH_MMR_TMU_TOCR);
	r = pte_map(adp, 0x7fff8000, 0x7fff8000+__PAGESIZE-1,
	            PROT_READ|PROT_NOCACHE, NULL, paddr, 0);
	if(r != EOK) goto fail2;

	return EOK;

fail2:
	cpu_vmm_mdestroy(prp);

fail1:
	return r;
}


void
cpu_vmm_mdestroy(PROCESS *prp) {
	uint32_t		**pdep;
	unsigned		num;
	unsigned		asid;
	ADDRESS			*adp;
	part_id_t	mpid = mempart_getid(prp, sys_memclass_id);
	memsize_t		memclass_pid_free = 0;

	adp = prp->memory;

	// flush tlb here to get rooms for others
	InterruptDisable();
	SPINLOCK(&asid_spin);
	asid = adp->cpu.asid;
	if(asid <= VM_ASID_BOUNDARY) {
#ifdef	VARIANT_smp
		/*
		 * We only flush asid when it is newly allocated
		 */
#else
		tlb_flush_asid(asid);
#endif
		asid_map[asid] = NULL;
		adp->cpu.asid = ~0;
	}
	SPINUNLOCK(&asid_spin);
	InterruptEnable();

	// free all the L2's in user space
	pdep = &adp->cpu.pgdir[0];
	// For a user process, some pte entries like timer for Clockcycles are
	// above user space boundry
	for(num = L1PAGEIDX(SH_P0BASE + SH_P0SIZE); num != 0; num--, pdep++) {
		if(*pdep != 0) {
			pa_free_paddr(ADDR_PAGE(SH_P2_TO_PHYS(*pdep)), __PAGESIZE, MEMPART_DECR(mpid, __PAGESIZE));
			memclass_pid_free += __PAGESIZE;
		}
	}
	// free the L1
	pa_free_paddr(ADDR_PAGE(SH_P2_TO_PHYS(adp->cpu.pgdir)), __PAGESIZE, MEMPART_DECR(mpid, __PAGESIZE));
	memclass_pid_free += __PAGESIZE;
	MEMCLASS_PID_FREE(prp, mempart_get_classid(mpid), memclass_pid_free);
}


#define MEG4	(4*1024*1024)


int
cpu_pte_split(uintptr_t vaddr, struct mm_pte_manipulate *data) {
	uint32_t                                *pde;
	uint32_t                                pte, *ptep;
	uint32_t                                **pgdir;
	uintptr_t                               pgsz;
	uintptr_t                               tsize;
	uintptr_t                               check;
	uintptr_t                               tvaddr;
	uintptr_t                               check_start;
	uintptr_t                               check_end;
	paddr_t                                 paddr;

	pgdir = data->adp->cpu.pgdir;

	pde = pgdir[L1PAGEIDX(vaddr)];
	if(pde != NULL) {
		pte = pde[L2PAGEIDX(vaddr)];
		if(PRESENT(pte)) {
			pgsz = SH_PTE_PGSIZE(pte);
			check_start = check = ROUNDDOWN(vaddr, pgsz);
			check_end = check + pgsz - 1;
			// double check this logic
			if((check < vaddr) || ((vaddr == data->start) && (check_end > data->end))) {
				uint32_t	new_flags;

				// We need to split the entries into smaller sizes.
				// For right now, just do 4K

				if(check_end > data->split_end) data->split_end = check_end;
				new_flags = (PTE_ATTR(pte) & ~SH_CCN_PTEL_SZ_M) | SH_CCN_PTEL_SZ0; // 4k page
				//kprintf("splitting %x from size %x\n", check, pgsz);

				// First, clear the page table entries and flush the TLBs
				tvaddr = check;
				tsize = pgsz;
				for( ;; ) {
					ptep = &pgdir[L1PAGEIDX(tvaddr)][L2PAGEIDX(tvaddr)];
					*ptep = 0;
					tsize -= __PAGESIZE;
					if(tsize == 0) break;
					tvaddr += __PAGESIZE;
				}
				tlb_flush_va(data->adp, check_start, check_end);

				// Put the new PTEs in place
				paddr = REAL_ADDR(pte);
				tvaddr = check;
				tsize = pgsz;
				for( ;; ) {
					ptep = &pgdir[L1PAGEIDX(tvaddr)][L2PAGEIDX(tvaddr)];
					*ptep = paddr | new_flags;
					tsize -= __PAGESIZE;
					if(tsize == 0) break;
					paddr += __PAGESIZE;
					tvaddr += __PAGESIZE;
				}
				// Since cpu_pte_split gets called multiple times,
				// this gives the upper level a chance to preempt
				if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
					return EINTR;
				}
			}
		}
	}
	return EOK;
}


int
cpu_pte_merge(struct mm_pte_manipulate *data) {
	uint32_t                **pgdir;
	uint32_t                *pde;
	uint32_t                *ptep, pte;
	uint32_t                *chk_ptep, chk_pte;
	uintptr_t               *pgszp;
	unsigned                pgsz;
	uintptr_t               run_start;
	uintptr_t               run_last;
	uintptr_t               chk_end;
	uintptr_t               chk_vaddr;
	uintptr_t               merge_start;
	uintptr_t               merge_end;
	paddr_t                 paddr;
	unsigned                pte_flags;
	unsigned                pte_paddr;

	run_start = run_last = 0; // Shut up GCC warning.

	pgdir = data->adp->cpu.pgdir;
	if(!(data->op & PTE_OP_MERGESTARTED)) {
		// look at the PTE's in front of data->start and see if they're
		// contiguous - we might be able to merge them in now.
		chk_vaddr = data->start;
		// try merging to the start of the largest available pagesize in
		// front of the manipulated region.
		merge_start = ROUNDDOWN(chk_vaddr, pgszlist[0]);
		chk_ptep = NULL;
		for( ;; ) {
			pde = pgdir[L1PAGEIDX(chk_vaddr)];
			if(pde == NULL) break;
			ptep = &pde[L2PAGEIDX(chk_vaddr)];
			pte = *ptep;
			if(!PRESENT(pte)) break;
			if(chk_ptep != NULL) {
				pgsz = SH_PTE_PGSIZE(pte);
				chk_vaddr = chk_vaddr & ~(pgsz - 1);
				ptep = &pgdir[L1PAGEIDX(chk_vaddr)][L2PAGEIDX(chk_vaddr)];
				pte = *ptep;
				chk_pte = *chk_ptep;
				if(PTE_PERMS(pte) != PTE_PERMS(chk_pte)) break;
				if((REAL_ADDR(pte) + pgsz) != REAL_ADDR(chk_pte)) break;
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
	merge_end = ROUNDUP(data->split_end + 1, pgszlist[0]) - 1;
	do {
		chk_ptep = NULL;
		for( ;; ) {
			if(data->start >= merge_end) break;
			pde = pgdir[L1PAGEIDX(data->start)];
			if(pde == NULL) break;
			ptep = &pde[L2PAGEIDX(data->start)];
			pte = *ptep;
			if(!PRESENT(pte)) break;
			if(chk_ptep == NULL) {
				run_start = data->start;
			} else {
				chk_pte = *chk_ptep;
				if(PTE_PERMS(pte) != PTE_PERMS(chk_pte)) break;
				if((REAL_ADDR(chk_pte) + SH_PTE_PGSIZE(chk_pte)) != REAL_ADDR(pte)) break;
			}
			run_last = data->start;
			data->start += SH_PTE_PGSIZE(pte);
			if(data->start == 0) data->start = ~(uintptr_t)0;
			chk_ptep = ptep;
		}
		if(chk_ptep != NULL) {
			// We've got a run of PTE's with perms that are the same and the
			// paddrs are contiguous. Start building up larger page sizes
			// based on the vaddr/paddr alignment
			while(run_start < run_last) {
				ptep = &pgdir[L1PAGEIDX(run_start)][L2PAGEIDX(run_start)];
				pte = *ptep;
				paddr = REAL_ADDR(pte);
				pgszp = pgszlist;
				// Find a page size that works
				do {
					pgsz = *pgszp++;
				} while((pgsz > ((run_last-run_start)+__PAGESIZE))
				        || ((run_start & (pgsz-1)) != 0)
				        || ((paddr & (pgsz-1)) != 0));
				if(pgsz > SH_PTE_PGSIZE(pte)) {
					pte_paddr = REAL_ADDR(paddr);
					pte_flags = PTE_PGSIZE_TO_ATTR(pgsz) | PTE_PERMS(pte);

					// merge the entries.

					//kprintf("merging %x to size %x pte_f %x\n", run_start, pgsz, pte_flags);
					chk_vaddr = run_start;
					chk_end = run_start + pgsz;
					for(;;) {
						*ptep = 0;
						chk_vaddr += __PAGESIZE;
						if(chk_vaddr == chk_end) break;
						ptep = &pgdir[L1PAGEIDX(chk_vaddr)][L2PAGEIDX(chk_vaddr)];
					}
					tlb_flush_va(data->adp, run_start, chk_end - __PAGESIZE);

					// Now actually do the merging of the PTE's into
					// a big page.
					chk_vaddr = run_start;
					for(;;) {
						ptep = &pgdir[L1PAGEIDX(chk_vaddr)][L2PAGEIDX(chk_vaddr)];
						*ptep = pte_paddr | pte_flags;
						chk_vaddr += __PAGESIZE;
						if(chk_vaddr == chk_end) break;
					}
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
	uint32_t				*ptep;
	uint32_t				*pde;
	uint32_t				pte;
	uint32_t				orig_pte;
	uint32_t				bits;
	ADDRESS					*adp;
	unsigned				L1PAGEIDX;
	uintptr_t				start;
	int						need_flush;
	int						r;

	// Assume alignment has been checked by the caller

	adp = data->adp;

	if(data->op & PTE_OP_BAD) {
		bits = SH_CCN_PTEL_PR(3);
	} else {
		bits = 0;
		if ( __cpu_flags & SH_CPU_FLAG_SHM_SPECIAL_BITS ) {
			if(data->shmem_flags & SHMCTL_HAS_SPECIAL) bits |= data->special;
		}
		if(!(data->prot & PROT_NOCACHE)) bits |= SH_CCN_PTEL_C;
		if(data->prot & PROT_WRITE) {
			if(data->op & PTE_OP_TEMP) {
				bits |= SH_CCN_PTEL_V|SH_CCN_PTEL_D|SH_CCN_PTEL_PR(1);
			} else {
				bits |= SH_CCN_PTEL_V|SH_CCN_PTEL_D|SH_CCN_PTEL_PR(3);
			}
		} else if(data->prot & (PROT_READ|PROT_EXEC)) {
			if(data->op & PTE_OP_TEMP) {
				bits |= SH_CCN_PTEL_V|SH_CCN_PTEL_PR(0);
			} else {
				bits |= SH_CCN_PTEL_V|SH_CCN_PTEL_PR(2);
			}
		}
	}

	r = EOK;
	need_flush = 0;
	start = data->start;
	for( ;; ) {
		if(data->start >= data->end) break;

		L1PAGEIDX = L1PAGEIDX(data->start);
		pde = adp->cpu.pgdir[L1PAGEIDX];
		if(pde == NULL) {
			if(!(data->op & (PTE_OP_MAP|PTE_OP_PREALLOC|PTE_OP_BAD))) {
				//Move vaddr to next page directory
				data->start = (data->start + MEG4) & ~(MEG4-1);
				continue;
			}

			pde = alloc_pgentry(adp ? object_from_data(adp, address_cookie) : NULL);
			if(pde == NULL) {
				if(need_flush) {
					tlb_flush_va(adp, start, data->start-1);
				}
				return ENOMEM;
			}
			adp->cpu.pgdir[L1PAGEIDX] = pde;
		}
		if(data->op & PTE_OP_PREALLOC) {
			//Move vaddr to next page directory
			data->start = (data->start + MEG4) & ~(MEG4 - 1);
			continue;
		}
		ptep = &pde[L2PAGEIDX(data->start)];
		orig_pte = *ptep;
		if(data->op & (PTE_OP_MAP|PTE_OP_BAD)) {
			pte = data->paddr | bits | SH_CCN_PTEL_SZ0; // size = 4K
		} else if(data->op & PTE_OP_UNMAP) {
			pte = 0;
		} else if(PRESENT(orig_pte)) {
			// PTE_OP_PROT
			pte = REAL_ADDR(orig_pte) | bits | (orig_pte & SH_CCN_PTEL_SZ_M); // size from original
		} else {
			// Don't change PTE permissions if we haven't mapped the page yet..
			pte = orig_pte;
		}
		*ptep = pte;
		if((pte != orig_pte) && (orig_pte != 0)) {
			need_flush = 1;
		}
		data->start += __PAGESIZE;
		data->paddr += __PAGESIZE;
		if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			r = EINTR;
			break;
		}
	}
	if(need_flush) {
		tlb_flush_va(adp, start, data->start-1);
	}
	return r;
}


unsigned
cpu_vmm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *paddrp, size_t *lenp) {
	uint32_t				pte;
	uint32_t				*pde;
	ADDRESS					*adp;
	unsigned				offset;
	unsigned				prot;
	struct sh4_tlb 			*perm;
	unsigned				mask;

	/* piece of cake for P1, P2 addresses */
	if(SH_IS_P1(vaddr) || SH_IS_P2(vaddr)) {
		// Assuming P1 & P2 are the same size
		offset = vaddr & (SH_P1SIZE - 1);
		*paddrp = offset;
		if(lenp != NULL) *lenp = SH_P1SIZE - offset;
		return PROT_READ|PROT_WRITE|PROT_EXEC;
	}

	offset = ADDR_OFFSET(vaddr);

	// Check for system, cpu pages...
	if(ADDR_PAGE(vaddr) == SYSP_ADDCOLOR(VM_CPUPAGE_ADDR, SYSP_GETCOLOR(_cpupage_ptr))) {
		*paddrp = SH_P1_TO_PHYS((uintptr_t)_cpupage_ptr) | offset;
		if(lenp != NULL) *lenp = __PAGESIZE - offset;
		return PROT_READ;
	}
	if(ADDR_PAGE(vaddr) == SYSP_ADDCOLOR(VM_SYSPAGE_ADDR, SYSP_GETCOLOR(_syspage_ptr))) {
		*paddrp = SH_P1_TO_PHYS((uintptr_t)_syspage_ptr) | offset;
		if(lenp != NULL) *lenp = __PAGESIZE - offset;
		return PROT_READ|PROT_EXEC;
	}

	if(prp == NULL) crash();
	adp = prp->memory;
	if(adp == NULL) crash();

	pde = adp->cpu.pgdir[L1PAGEIDX(vaddr)];
	if((pde == NULL) || !PRESENT((pte = pde[L2PAGEIDX(vaddr)]))) {
		/* not mapped, check for perm mappings */
		if((perm = perm_map_check(vaddr, 0, 0)) == NULL) {
			return PROT_NONE;
		}
		pte = perm->data;
	}

	mask = SH_PTE_PGSIZE_MASK(pte);
	*paddrp = (pte & mask) | (vaddr & ~mask);

	if(lenp != NULL) {
		unsigned pgsize = SH_PTE_PGSIZE(pte);
		*lenp = pgsize - offset;
	}
	prot = MAP_PHYS; // So we won't return PROT_NONE
	if(IS_PAGEREADABLE(pte)) prot |= PROT_READ|PROT_EXEC;
	if(IS_PAGEWRITEABLE(pte)) prot |= PROT_WRITE;
	return prot;
}


/*
 * tlb_flush_entry()
 *	flush an entry in data tlb
 *	no need to disable interrupt
 */
void
tlb_flush_entry(ADDRESS *adp, unsigned entry) {
	unsigned ptr;

	ptr = (entry<<8) + SH4_UTLB_ADDRESS_ARRAY;
	sh_tlb_out32(ptr, 0);

}

__SRCVERSION("$IQ: cpu_vmm.c,v 1.28 $");
