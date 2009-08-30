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

#ifdef PTEHACK_SUPPORT
static struct mips_ptehack_entry        *ptehack_ptr;
static unsigned                         ptehack_num;
#endif


static struct r4k_tlb			*permanent_tlb_entries;
static unsigned					num_permanent_tlb;

#if defined(VARIANT_smp)
	static uintptr_t   			user_cpupage;
	static unsigned    			cpupage_spacing;
#endif

static uintptr_t				colour_va = VA_INVALID;
static uintptr_t				colour_pte;
static uintptr_t				colour_base;


/*
 * perm_map_check: check for a fault on one of the permanent entries and
 *      (optionally) put it back in the TLB if need be.
 */
static struct r4k_tlb *
perm_map_check(uintptr_t badva, unsigned write_fault, unsigned load_tlb) {
	int                        i;
	uintptr_t                  mask;
	struct r4k_tlb		       *perm;
	unsigned                   lo0;


	/* Check for one of the permanent mappings */
	for(i = num_permanent_tlb-1; i >= 0; --i) {
		perm = &permanent_tlb_entries[i];
		mask = VPN_MASK(perm->pmask);
		if((perm->hi & mask) == (badva & mask)) {

			/* If a write fault and perm_mapping is R/O, early out */
			/* @@@ assumes entrylo0 and entrylo1 have same prot */
			if(write_fault && !(perm->lo0 & MIPS_TLB_WRITE)) return 0;

			/* Found a permanent entry! Slap it in and get out */
			lo0 = perm->lo0;
#if defined(VARIANT_smp)
			if((badva & mask) == user_cpupage) {
				//
				// For a user cpupage reference, we have to adjust
				// the physical address to point at the proper memory
				// for the CPU that we're running on.
				//
				lo0 += cpupage_spacing * RUNCPU;
			}
#endif
			if(load_tlb) {
				r4k_update_tlb(perm->hi, lo0, perm->lo1, perm->pmask);
			}
			return perm;
		}
	}
	return NULL;
}


uintptr_t
perm_map_init(void) {
	unsigned				i;
	unsigned				j;
	struct r4k_tlb			*perm;
	uintptr_t				end;
	uintptr_t				free_base = MIPS_R4K_K3BASE;
	struct r4k_tlb  		tlb;

	/*
	 * Collect all the permanent TLB mappings.
	 */

	j = 0;
	for(i = 0; i < num_tlbs; ++i) {
		r4k_gettlb(&tlb, i << TLB_IDX_SHIFT);
		if((tlb.lo0|tlb.lo1) & TLB_VALID) {
			++j;
		}
	}
	num_permanent_tlb = j;
	perm = malloc(sizeof(*permanent_tlb_entries)*num_permanent_tlb);
	if(perm == NULL) {
		 crash();
	}
	permanent_tlb_entries = perm;
	for(i = 0; i < num_tlbs; ++i) {
		r4k_gettlb(&tlb, i << TLB_IDX_SHIFT);
		if((tlb.lo0|tlb.lo1) & TLB_VALID) {
			*perm = tlb;
			end = (perm->hi & ~TLB_HI_ASIDMASK) + (~VPN_MASK(perm->pmask) + 1);
			if(end > free_base) free_base = end;
			++perm;
		}
	}

#if defined(VARIANT_smp)
	//
	// Remember where the user address for the cpupage is and
	// the (adjusted) spacing between cpupages on an SMP system
	// for use in perm_map_check().
	//
	{
		struct system_private_entry *pp;
		
		pp = SYSPAGE_ENTRY(system_private);
		user_cpupage = (uintptr_t)pp->user_cpupageptr;
		cpupage_spacing = pp->cpupage_spacing >> (PG_PFNSHIFT-MIPS_TLB_LO_PFNSHIFT);
	}
#endif

	return free_base;
}


uintptr_t	
colour_init(uintptr_t free_base, unsigned colour_size) {
	free_base = ROUNDUP(free_base, colour_size);
	colour_base = free_base;
	return free_base + colour_size;
}


static void
colour_reload() {
	uint32_t	tlblo0;
	uint32_t	tlblo1;
	uintptr_t	tlbhi;

#if defined(VARIANT_r3k) 
	tlbhi = colour_va & ~(__PAGESIZE-1);
	tlblo0 = colour_pte;
	tlblo1 = 0;
#else	
	tlbhi = colour_va & ~(__PAGESIZE*2-1);
	if(colour_va & __PAGESIZE) {
		tlblo1 = colour_pte;
		tlblo0 = 0;
	} else {
		tlblo0 = colour_pte;
		tlblo1 = 0;
	}
#endif	

	tlbhi |= (getcp0_tlb_hi() & TLB_HI_ASIDMASK);

	r4k_update_tlb(tlbhi, tlblo0, tlblo1, pgmask_4k);
}

struct kerargs_colour_operation {
	void 		(*rtn)(void *);
	unsigned 	colour;
	paddr_t		paddr;
};


static void
kerext_colour_operation(void *data) {
	struct kerargs_colour_operation *kap = data;

	colour_operation(kap->rtn, kap->colour, kap->paddr);
}

void
colour_operation(void (*rtn)(void *), unsigned colour, paddr_t paddr) {
	if(!KerextAmInKernel()) {
		struct kerargs_colour_operation data;

		data.rtn = rtn;
		data.colour = colour;
		data.paddr = paddr;

		__Ring0(kerext_colour_operation, &data);
		return;
	}

	if(colour_va != VA_INVALID) {
		crash();
	}

	KerextLock();

	colour_pte = TLB_WRITE | TLB_VALID | TLB_CACHEABLE
					| ((uint32_t)(paddr >> pfn_topshift) & TLB_LO_PFNMASK);
	colour_va = colour_base + (colour << ADDR_OFFSET_BITS);

	colour_reload();

	rtn((void *)colour_va);

	colour_va = VA_INVALID;
}


/*
 * tlb_flush_va()
 *       Flush TLB associated with a particular virtual address range
 */
static void 
tlb_flush_va(ADDRESS *adp, uintptr_t va) {
	unsigned	i;
	unsigned	mycpu = RUNCPU;

	//WIRE: check for global mapping?
	va &= ~ADDR_OFFSET_MASK;
	for(i = 0; i < NUM_PROCESSORS; ++i) {
		if(i == mycpu) {
			(void) r4k_flush_tlb(va | (adp->cpu.asid[i] << TLB_HI_ASIDSHIFT));
		} else {
			atomic_set(&adp->cpu.pending_asid_purges, 1 << i);
			SENDIPI(i, IPI_TLB_FLUSH);
		}
	}
}


int 
cpu_vmm_fault(struct fault_info *info) {
	uintptr_t		vaddr;
	unsigned		sigcode;
	pte_t			*pde;
	uint32_t		pte_lo;
	struct xfer_map	*xfer;
	PROCESS			*prp;
	ADDRESS			*adp;

	sigcode = info->sigcode;
	vaddr = info->vaddr;

	/* Do this early so that kprintf's work */
	if(SIGCODE_SIGNO(sigcode) == SIGSEGV) {
		if(perm_map_check(vaddr, FAULT_ISWRITE(sigcode), 1) != NULL) {
			return 1;
		}
	}
	if(colour_va == ADDR_PAGE(vaddr)) {
		colour_reload();
		return 1;
	}

#if 0 
	//RUSH3: Re-enable when we support global mappings again.
	//RUSH3: Should only do this if we aren't in Supervisor mode already
#if !defined(VARIANT_r3k)
	if((mem->flags & MAP_GLOBAL) && IS_GLOBAL_SUPV_ADDR(mem->addr, mem->size)) {
		/*
		 * This is a global mapping 
		 */
		info->cpu.regs[MIPS_CREG(MIPS_REG_SREG)] &= ~MIPS_SREG_KSU;
		info->cpu.regs[MIPS_CREG(MIPS_REG_SREG)] |= MIPS_SREG_MODE_SUPER;
		return 1;
	}
#endif		
#endif

	prp = info->prp;
	adp = prp->memory;
	pde = adp->cpu.pgdir[L1IDX(vaddr)];
	if(pde != NULL) {
		pte_lo = pde[L2IDX(vaddr)].lo;
		if((pte_lo & TLB_VALID)) {
			if(!FAULT_ISWRITE(sigcode) || (pte_lo & TLB_WRITE)) {
				pte_t 				*pt;
				uintptr_t			tlbhi;

				//RUSH3: Gather some stats on how often this is being
				//RUSH3: done - we might be able improve performance
				//RUSH3: by doing more TLB flushing in cpu_pte_manipulate()

				/*
				 * Entry in page table already- must be a tlb
				 * invalid exception- there was a tlb entry for this
				 * address but it was invalid. For that situation
				 * the cpu doesn't go through the tlbmiss code, and
				 * we'll end up here. Just drop the entry into the
				 * tlb and be done with it- no need to involve procnto.
				 */
				vaddr = ADDR_PAGE(vaddr);

				pt = &adp->cpu.pgdir[L1IDX(vaddr)][L2IDX(vaddr)];

#if defined(VARIANT_r3k)
				#define OTHER_PTE_LO	0
#else
				#define	OTHER_PTE_LO	pt[1].lo
				{
					pte_t		*pt2;

					if(vaddr & __PAGESIZE) {
						pt2 = pt - 1;
					} else {
						pt2 = pt + 1;
					}
					if(!PTE_PRESENT(pt2->lo)) {
						pt2->lo = pt->lo & TLB_GLOBAL;
						pt2->pm = pt->pm;
					}
					if(pt2 < pt) pt = pt2;
				}
#endif

				/*
				 * Set up the part of the TLB entry common to the pair of
				 * TLB low entries.
				 */

				tlbhi = (vaddr & VPN_MASK(pt->pm)) | (adp->cpu.asid[RUNCPU] << TLB_HI_ASIDSHIFT);
				/*
				 * Plop the entry into the tlb.
				 */
				r4k_update_tlb(tlbhi, pt->lo, OTHER_PTE_LO, pt->pm);
				return 1;
			}
		} else if(pte_lo & TLB_WRITE) {
			// PTE marked as 'bad' by cpu_pte_manipulate(PTE_OP_BAD)
			return -2;
		}
	}

	if((vaddr >= kern_xfer_base) && (vaddr <= kern_xfer_end)) {
		if(sigcode & SIGCODE_INXFER) {
			/*
			 * fault happened while doing a transfer- faulted
			 * in the region of memory belonging to the
			 * non-active process. Get the non-active process and
			 * the base virtual address (not the kernel address).
			 */
			xfer  = &xfer_tbl[(vaddr - kern_xfer_base) / KERN_XFER_SLOT_SIZE];
			info->vaddr = vaddr - xfer->diff;
			info->prp = xfer->prp;
		}
	}
	return 0;
}


int
cpu_vmm_mcreate(PROCESS *prp) {
	ADDRESS						*adp;
	uintptr_t                   start;
	uintptr_t                   end;
	struct system_private_entry *pp;
	void						*vaddr;
	size_t						size;
	struct map_set				ms;
	struct map_set				repl;
	int							r;
	unsigned					i;

	wire_mcreate(prp);

	/*
	 * allocate and zero the level1 page table.
	 */
	r = vmm_mmap(NULL, 0, __PAGESIZE*2, PROT_READ | PROT_WRITE, 
				MAP_PRIVATE|MAP_ANON|MAP_PHYS, 0, 0, __PAGESIZE, 0, NOFD, &vaddr, &size,
				mempart_getid(prp, sys_memclass_id));
	if(r != EOK) goto fail1;

	CPU_ZERO_PAGE(vaddr, __PAGESIZE*2, 0);
	adp = prp->memory;
	adp->cpu.pgdir = vaddr;
	for(i = 0; i < NUM_PROCESSORS; ++i) {
		adp->cpu.asid[i] = ~0;
	}

	/*
	 * reserve the virtual memory for the
	 * user view of the syspage/cpupage(s).
	 */
	pp = SYSPAGE_ENTRY(system_private);
	start = (uintptr_t)pp->user_syspageptr;
	if(start < (uintptr_t)pp->user_cpupageptr) {
		end = (uintptr_t)pp->user_cpupageptr + sizeof(*pp->user_cpupageptr);
	} else {
		end = start + _syspage_ptr->total_size;
		start = (uintptr_t)pp->user_cpupageptr;
	}
	size = ROUNDUP((end - start) + 1, __PAGESIZE*2);
	r = map_create(&ms, &repl, &adp->map, start, size, 0, MAP_FIXED);
	if(r != EOK) goto fail2;
	r = map_add(&ms);
	if(r != EOK) goto fail3;

	ms.first->extra_flags |= EXTRA_FLAG_SPECIAL;

	adp->rlimit.vmem += size;

	//RUSH3: Add the page table entries for sys/cpu pages? Beware of SMP issues

	return EOK;

fail3:
	map_destroy(&ms);

fail2:
	vmm_munmap(NULL, (uintptr_t)vaddr, __PAGESIZE*2, 0, mempart_getid(prp, sys_memclass_id));
	
fail1:
	return r;
}


void
cpu_vmm_mdestroy(PROCESS *prp) {
	ADDRESS     		*adp;
	struct pa_quantum	*curr;
	struct pa_quantum	*next;
	unsigned			i;
	part_id_t		mpid;
	memsize_t			memclass_pid_free = 0;

	adp = prp->memory;

	/*
	 * Make sure our transfer cache doesn't refer to this address
	 * space, which is going away.
	 */
	xfer_cache_kill(adp);

	//Since mdestroy is called while in the kernel, the asid_map won't
	//flip around while we're clearing it.
	for(i = 0; i < NUM_PROCESSORS; ++i) {
		int asid = adp->cpu.asid[i];

		if(VALID_ASID(asid)) {
			if(i == RUNCPU) {
				gbl_asid_map[i]->map[asid] = NULL;
				if(icache_flags & CACHE_FLAG_VIRT_TAG) {
					r4k_purge_icache_full();
				}
			} else {
				gbl_asid_map[i]->map[asid] = PENDING_PURGE_ADDRESS;
			}
		}
	}

	// release all the L2 page tables
	curr = adp->cpu.l2_list;
	mpid = mempart_getid(prp, sys_memclass_id);
	while(curr != NULL) {
		next = curr->u.inuse.next;
		pa_free(curr, 1, MEMPART_DECR(mpid, NQUANTUM_TO_LEN(1)));
		memclass_pid_free += NQUANTUM_TO_LEN(1);
		curr = next;
	}
	MEMCLASS_PID_FREE(prp, mempart_get_classid(mpid), memclass_pid_free);

	vmm_munmap(NULL, (uintptr_t)adp->cpu.pgdir, __PAGESIZE*2, 0, mpid);
}


void
pgszlist_init(unsigned pgsz) {
	uintptr_t		*pgszp = pgszlist;

	do {
		*pgszp++ = pgsz;
		pgsz >>= 2;
	} while(pgsz >= __PAGESIZE);
}

#define PTE_PGSIZE(p)		PGMASK_TO_PGSIZE((p)->pm)
#define PTE_PERMS(p)		((p)->lo & ~TLB_LO_PFNMASK)
#define MAKE_PTE_PADDR(p)	((uint32_t)((p) >> pfn_topshift) & TLB_LO_PFNMASK)


#if CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES != VPS_NONE
int
cpu_pte_split(uintptr_t vaddr, struct mm_pte_manipulate *data) {
	pte_t					*pde;
	pte_t					*ptep;
	pte_t					**pgdir;
	uintptr_t				pgsz2;
	uintptr_t				check;
	uintptr_t				check_end;

	pgdir = data->adp->cpu.pgdir;

	pde = pgdir[L1IDX(vaddr)];
	if(pde != NULL) {
		ptep = &pde[L2IDX(vaddr)];
		if(PTE_PRESENT(ptep->lo)) {
			// Multiply the actual page size by two to deal with
			// the MIPS TLB format of having one ENTRYHI control two
			// ENTRYLO registers.
			pgsz2 = PTE_PGSIZE(ptep)*2;
			if(pgsz2 > (__PAGESIZE*2)) {
				check = ROUNDDOWN(vaddr, pgsz2);
				check_end = check + pgsz2 - 1;
				if((check < vaddr) || ((vaddr == data->start) && (check_end > data->end))) {
					unsigned	new_flags0;
					paddr_t		paddr0;
					unsigned	new_flags1;
					paddr_t		paddr1;
					uintptr_t	pgsz;

					// We need to split the entries into smaller sizes.
					// For right now, just do 4K

					if(check_end > data->split_end) data->split_end = check_end;
					if(vaddr & __PAGESIZE) {
						// We picked up the odd ENTRYLO, need the even one.
						--ptep;
					}
					pgsz = pgsz2 / 2;
					new_flags0 = ptep[0].lo & ~TLB_LO_PFNMASK;
					paddr0 = PTE_PADDR(ptep[0].lo);	
					new_flags1 = ptep[1].lo & ~TLB_LO_PFNMASK;
					paddr1 = PTE_PADDR(ptep[1].lo);	
//kprintf("splitting %x from size %x\n", check, pgsz);
					// Temporarily unmap the pages that we're merging. 
					// That way nobody gets an inconsistent view of the set.
					{
						uintptr_t	um = check;
						uintptr_t	um_end = check + pgsz2;
						do {
							ptep = &pgdir[L1IDX(um)][L2IDX(um)];
							ptep[0].lo = 0;
							ptep[1].lo = 0;
							um += __PAGESIZE*2;
						} while(um != um_end);
					}
					tlb_flush_va(data->adp, vaddr);
					for( ;; ) {
						ptep = &pgdir[L1IDX(check)][L2IDX(check)];
						ptep[0].pm = pgmask_4k;
						ptep[0].lo = new_flags0 | MAKE_PTE_PADDR(paddr0);
						ptep[1].pm = pgmask_4k;
						ptep[1].lo = new_flags0 | MAKE_PTE_PADDR(paddr0+__PAGESIZE);
						pgsz2 -= __PAGESIZE*2;
						if(pgsz2 == 0) break;
						paddr0 += __PAGESIZE*2;
						if(pgsz2 == pgsz) {
							// half way done, switch to odd set of PTE's
							new_flags0 = new_flags1;
							paddr0 = paddr1;
						}
						check += __PAGESIZE*2;
					}
					// Since cpu_pte_split gets called multiple times, 
					// this gives the upper level a chance to preempt
					if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
						return EINTR;
					}
				}
			}
		}
	}
	if(!(data->op & PTE_OP_TEMP) 
		&& (data->op & PTE_OP_UNMAP)	
			&& (data->shmem_flags & SHMCTL_HIGHUSAGE)) {
		// Have to go into the merge code to clean up any wired entries
		data->op |= PTE_OP_FORCEMERGE;
	}
	return EOK;
}

int
cpu_pte_merge(struct mm_pte_manipulate *data) {
	pte_t			**pgdir;
	pte_t			*pde;
	pte_t			*ptep;
	pte_t			*chk_ptep;
	uintptr_t		*pgszp;
	unsigned		pgsz;
	unsigned		pgsz2;
	uintptr_t		run_start;
	uintptr_t		run_last;
	uintptr_t		chk_end;
	uintptr_t		chk_vaddr;
	uintptr_t		merge_start;
	uintptr_t		merge_end;
	paddr_t			paddr;
	unsigned		pte_lo0;
	unsigned		pte_lo1;
	unsigned		pte_pm;

	// This code actually doesn't build as many big pages as it could.
	// Because the MIPS TLB has one ENTRYHI controlling two ENTRYLO's,
	// we require the vaddr alignment and contiguous length to be twice 
	// the amount of the actual page size. In reality, we don't need
	// the even/odd ENTRYLO's to be physically contiguous, nor have
	// the same permissions. I can't see a nice way of figuring out
	// when that would be possible though.

	run_start = run_last = 0; // Shut up GCC warning.

	pgdir = data->adp->cpu.pgdir;
	if(!(data->op & PTE_OP_MERGESTARTED)) {
		// look at the PTE's in front of data->start and see if they're
		// contiguous - we might be able to merge them in now. 
		chk_vaddr = data->start;
		// try merging to the start of the largest available pagesize in
		// front of the manipulated region.
		merge_start = ROUNDDOWN(chk_vaddr, pgszlist[0]*2);
		chk_ptep = NULL;
		for( ;; ) {
			pde = pgdir[L1IDX(chk_vaddr)];
			if(pde == NULL) break;
			ptep = &pde[L2IDX(chk_vaddr)];
			if(!PTE_PRESENT(ptep->lo)) break;
			if(chk_ptep != NULL) {
				pgsz = PTE_PGSIZE(ptep);
				// If we've got a big page, the even/odd PTE's are
				// tied together (see comment at begining of function),
				// so we want to pretend that the page size is twice the
				// size it really is.
				if(pgsz > __PAGESIZE) pgsz *= 2;
				chk_vaddr = chk_vaddr & ~(pgsz - 1);
				ptep = &pgdir[L1IDX(chk_vaddr)][L2IDX(chk_vaddr)];
				if(PTE_PERMS(ptep) != PTE_PERMS(chk_ptep)) break;
				if((PTE_PADDR(ptep->lo) + pgsz) != PTE_PADDR(chk_ptep->lo)) break;
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
	merge_end = ROUNDUP(data->split_end + 1, pgszlist[0]*2) - 1;
	do {
		pgsz = 0;
		chk_ptep = NULL;
		for( ;; ) {
			if(data->start >= merge_end) break;
			pde = pgdir[L1IDX(data->start)];
			if(pde == NULL) break;
			ptep = &pde[L2IDX(data->start)];
			if(!PTE_PRESENT(ptep->lo)) break;
			if(chk_ptep == NULL) {
				run_start = data->start;
			} else {
				if(PTE_PERMS(ptep) != PTE_PERMS(chk_ptep)) break;
				if((PTE_PADDR(chk_ptep->lo) + pgsz) != PTE_PADDR(ptep->lo)) break;
			}
			run_last = data->start;
			pgsz = PTE_PGSIZE(ptep);
			if(pgsz > __PAGESIZE) pgsz *= 2;
			data->start += pgsz;
			chk_ptep = ptep;
		}
		
		if(chk_ptep != NULL) {
			// We've got a run of PTE's with perms that are the same and the
			// paddrs are contiguous. Start building up larger page sizes
			// based on the vaddr/paddr alignment
			while(run_start < run_last) {
				ptep = &pgdir[L1IDX(run_start)][L2IDX(run_start)];
				paddr = PTE_PADDR(ptep->lo);
				pgszp = pgszlist;
				// Find a page size that works
				do {
					pgsz = *pgszp++;
					pgsz2 = pgsz * 2;
					if(pgsz == 0) break;
				} while((pgsz2 > ((run_last-run_start)+__PAGESIZE)) 
					  || ((run_start & (pgsz2-1)) != 0)
					  || ((paddr & (pgsz-1)) != 0));

				if(pgsz > PTE_PGSIZE(ptep)) {
					pte_lo0 = MAKE_PTE_PADDR(paddr)      | (ptep->lo & ~TLB_LO_PFNMASK);
					pte_lo1 = MAKE_PTE_PADDR(paddr+pgsz) | (ptep->lo & ~TLB_LO_PFNMASK);
					pte_pm = ((pgsz2 - 1) & ~(__PAGESIZE*2-1)) | pgmask_4k;

					// merge the entries.

//kprintf("merging %x to size %x\n", run_start, pgsz);
					// Temporarily unmap and flush the TLB for the pages that 
					// we're merging. That way nobody gets an inconsistent
					// view of the set.
					chk_vaddr = run_start;
					chk_end = run_start + pgsz2;
					for(;;) {
						ptep[0].lo = 0;
						ptep[1].lo = 0;
						tlb_flush_va(data->adp, chk_vaddr);
						chk_vaddr += __PAGESIZE*2;
						if(chk_vaddr == chk_end) break;
						ptep = &pgdir[L1IDX(chk_vaddr)][L2IDX(chk_vaddr)];
					}
					// Now actually do the merging of the PTE's into
					// a big page.
					chk_vaddr = run_start;
					for(;;) {
						ptep = &pgdir[L1IDX(chk_vaddr)][L2IDX(chk_vaddr)];
						ptep[0].pm = pte_pm;
						ptep[1].pm = pte_pm;
						ptep[0].lo = pte_lo0;
						ptep[1].lo = pte_lo1;
						chk_vaddr += __PAGESIZE*2;
						if(chk_vaddr == chk_end) break;
					}
				} else if(pgsz == 0) {
					// For the run_start increment below...
					pgsz2 = __PAGESIZE;
				}
				run_start += pgsz2;
			}
		} else {
			data->start += __PAGESIZE;
		}
		if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			return EINTR;
		}
	} while(data->start < data->split_end);
	if(!(data->op & PTE_OP_TEMP) && (data->shmem_flags & SHMCTL_HIGHUSAGE)) {
		wire_check(data);
	}
	return EOK;
}
#endif


#define L1SIZE	(1 << PT_L1SHIFT)

int 
cpu_pte_manipulate(struct mm_pte_manipulate *data) {
	pte_t				*ptep;
	pte_t				*pde;
	uint32_t			pte_lo;
	uint32_t			orig_pte_lo;
	uint32_t			bits;
	ADDRESS				*adp;
	unsigned			l1idx;
	struct pa_quantum	*pq;
	unsigned			pa_status;
	part_id_t			mpid;
	PROCESS 			*prp;

	// Assume alignment has been checked by the caller

	adp = data->adp;

	if(data->op & PTE_OP_BAD) {
		// A special bit pattern that is recognized by cpu_vmm_fault()
		bits = TLB_WRITE;
	} else {
		if(data->shmem_flags & SHMCTL_HAS_SPECIAL) {
			if(data->special > TLB_MAX_SPECIAL) {
				return EINVAL;
			}
			bits = data->special << MIPS_TLB_LO_CSHIFT;
		} else if(data->prot & PROT_NOCACHE) {
			bits = TLB_NOCACHE;
		} else {
			bits = TLB_CACHEABLE;
		}
		if(data->prot & (PROT_READ|PROT_EXEC)) bits |= TLB_VALID;
		if(data->prot & PROT_WRITE) bits |= TLB_VALID|TLB_WRITE;
	}

	prp = adp ? object_from_data(adp, address_cookie) : NULL;
	mpid = mempart_getid(prp, sys_memclass_id);
	for( ;; ) {
		if(data->start >= data->end) return EOK;

		l1idx = L1IDX(data->start);
		pde = adp->cpu.pgdir[l1idx];
		if(pde == NULL) {
			memsize_t  resv = 0;
			
			if(!(data->op & (PTE_OP_MAP|PTE_OP_PREALLOC|PTE_OP_BAD))) {
				//Move vaddr to next page directory
				data->start = (data->start + L1SIZE) & ~(L1SIZE-1);
				continue;
			}
			if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
				return ENOMEM;
			}
			pq = pa_alloc(__PAGESIZE, __PAGESIZE, PAQ_COLOUR_NONE, 0, &pa_status, restrict_proc, resv);
			if(pq == NULL) {
				MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
				return ENOMEM;
			}
			MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
			pq->flags |= PAQ_FLAG_SYSTEM;	
			pq->u.inuse.next = adp->cpu.l2_list;
			adp->cpu.l2_list = pq;
			pde = (void *)CPU_P2V(pa_quantum_to_paddr(pq));

			colour_set((uintptr_t)pde, pq, pq->run);

			if(pa_status & PAA_STATUS_NOT_ZEROED) {
				CPU_ZERO_PAGE(pde, __PAGESIZE, 0);
			}
			adp->cpu.pgdir[l1idx] = pde;
		}
		if(data->op & PTE_OP_PREALLOC) {
			//Move vaddr to next page directory
			data->start = (data->start + L1SIZE) & ~(L1SIZE - 1);
			continue;
		}
		ptep = &pde[L2IDX(data->start)];
		orig_pte_lo = ptep->lo;
		if(data->op & (PTE_OP_MAP|PTE_OP_BAD)) {
			pte_lo = MAKE_PTE_PADDR(data->paddr) | bits;
#if !defined(VARIANT_r3k)			
			switch((unsigned)(data->paddr >> 37)) {
			case 0x0:	
			case 0x7:
				break;
			default:
				// The Broadcom SB-1 core supports 40 bit physical addresses,
				// but we only have room for 38 in the PTE format (and
				// the official spec says that there are only 36 bits for
				// the PFN). We now put bits 36 and 37 into the PTE and
				// and sign extend out for bits 38 and 39.
				// That lets us represent paddrs of the form:
				//		0x0?????????
				//		0x1?????????
				//		0xe?????????
				//		0xf?????????
				// If the paddr doesn't look like that, we'll error out
				// here with a strange errno number so that users don't
				// think that the mapping actually worked.
				return E2BIG;
			}
#endif		
#ifdef PTEHACK_SUPPORT
			#error munging of pte should be happening here.
#endif
			ptep->pm = pgmask_4k;
		} else if(data->op & PTE_OP_UNMAP) {
			pte_lo = 0;
			ptep->pm = pgmask_4k;
		} else if(PTE_PRESENT(orig_pte_lo)) {
			// PTE_OP_PROT
			pte_lo = (orig_pte_lo & TLB_LO_PFNMASK) | bits;
		} else {
			// Don't change PTE permissions if we haven't mapped the page yet..
			pte_lo = orig_pte_lo;
		}
		ptep->lo = pte_lo;
		if((pte_lo != orig_pte_lo) && (orig_pte_lo & TLB_VALID)) {
			tlb_flush_va(adp, data->start);
		}
		data->start += __PAGESIZE;
		data->paddr += __PAGESIZE;
		if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) return EINTR;
	}
}


unsigned 
cpu_vmm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *paddrp, size_t *lenp) {
	struct r4k_tlb 	*perm;
	uint32_t		pte_lo;
	pte_t			*ptep;
	ADDRESS			*adp;
	unsigned		offset;
	unsigned		prot;
	unsigned		pgsize;

	/* piece of cake for kseg0, kseg1 addresses */
	if((vaddr >= MIPS_R4K_K0BASE) && (vaddr < (MIPS_R4K_K1BASE+MIPS_R4K_K1SIZE))) {
		// Assuming kseg0 & kseg1 are the same size
		offset = vaddr & (MIPS_R4K_K0SIZE - 1);
		*paddrp = offset;
		if(lenp != NULL) *lenp = MIPS_R4K_K0SIZE - offset;
		return PROT_READ|PROT_WRITE|PROT_EXEC;
	}

	CRASHCHECK(prp == NULL);
	adp = prp->memory;
	CRASHCHECK(adp == NULL);

	// This isn't really a loop, since we've got a break at the end and
	// will never get back to the top. It's the cleanest way I can
	// think of to express the code with out duplicating things though :-(
	for( ;; ) {
		ptep = adp->cpu.pgdir[L1IDX(vaddr)];
		if(ptep != NULL) {
			ptep += L2IDX(vaddr);
			pte_lo = ptep->lo;
			if(PTE_PRESENT(pte_lo)) {
				pgsize = PGMASK_TO_PGSIZE(ptep->pm);
				if(pgsize > __PAGESIZE) {
					// Because we scramble the PTE's up when dealing with
					// big pages so that the TLB miss handler can be quick,
					// the ptep that we got up above might not have the correct
					// paddr information in it for this address. Because of the
					// way cpu_pte_merge() works, we can get the PTE for the
					// start of big page and artificially multiply the page
					// size by 2. Then when we pull the paddr out of the PTE
					// everything works out.
					uintptr_t check = vaddr & ~(pgsize*2 - 1);

					pte_lo = adp->cpu.pgdir[L1IDX(check)][L2IDX(check)].lo;
					pgsize *= 2;
						
				}
				break;
			}
		}
		/* not mapped, check for perm mappings */
		perm = perm_map_check(vaddr, 0, 0);

		if(perm == NULL) return PROT_NONE;

		pgsize = PGMASK_TO_PGSIZE(perm->pmask);
#if defined(VARIANT_r3k)
		pte_lo = perm->lo0;
#else
		//have to decide if to use lo0 or lo1...
		pte_lo = (vaddr & pgsize) ? perm->lo1 : perm->lo0;
#endif
		break;
	}

	offset = vaddr & (pgsize - 1);
	*paddrp = PTE_PADDR(pte_lo) + offset;
	if(lenp != NULL) *lenp = pgsize - offset;
	prot = MAP_PHYS; // So we won't return PROT_NONE
	if(pte_lo & TLB_VALID) prot |= PROT_READ|PROT_EXEC;
	if(pte_lo & TLB_WRITE) prot |= PROT_WRITE;
	return prot;
}

__SRCVERSION("cpu_vmm.c $Rev: 201442 $");
