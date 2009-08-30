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

// A dummy cpu_mm_aspace for use until we allocate one for procnto 
static struct cpu_mm_aspace	dummy_aspace;
struct cpu_mm_aspace	*pgtbl_list = &dummy_aspace;

static struct pa_quantum	*sys_l2_list;


//
// We add 0x3fc0 on to the PDE paddr since the PDTP is stored
// twice in the last 8 entries of the last page directory.
// See the startup/lib/x86/processor.c file for details
// on why it's duplicated.
//
#define PDPT_OFFSET		0x3fc0

SMP_SPINVAR(static, mem_spin);

static struct pa_restrict	restrict_4G[] = {
	{restrict_4G+1, NULL, CPU_SYSTEM_PADDR_START,	CPU_SYSTEM_PADDR_END},
	{NULL, 			NULL, 0, 						~(paddr32_t)0}
};


void
pgtbl_init(void) {
	dummy_aspace.pgdir = (void *)_syspage_ptr->un.x86.pgdir[0];
	dummy_aspace.ptroot_paddr = rdpgdir();
}
	

int 
cpu_vmm_fault(struct fault_info *info) {
	pxe_t							*ptep;
	pxe_t							*pdep;
	uintptr_t						vaddr;
	unsigned						pte_flags;

	vaddr = info->vaddr;
	pdep = VTOPDIRP(vaddr);
	if((PXE_GET_FLAGS(pdep)&(X86_PTE_PRESENT|X86_PDE_PS)) == X86_PTE_PRESENT) {
		ptep = VTOPTEP(vaddr);
		pte_flags = PXE_GET_FLAGS(ptep);
		if(pte_flags & X86_PTE_PRESENT) {
			if(!(info->cpu.code & X86_FAULT_PAGELP)) {
				// Fault caused by page not present and page is now present.
				// Somebody else must have touched the page and gotten it 
				// allocated before we got here.
				return 1;
			}
		} else if(pte_flags & X86_PTE_USER2) {
			// This page has been marked as permanently bad
			return -2;
		}
	}

	if((vaddr >= MAP_BASE) && (vaddr < (MAP_BASE+MAP_SIZE))) {
		if(info->sigcode & SIGCODE_INXFER) {
			struct xfer_slots *slot = &xfer_slot[RUNCPU];

			if(slot->prp != NULL) {
				info->prp = (PROCESS *)slot->prp;
				if((vaddr >= slot->base0) && (vaddr < slot->base0 + (1 << (pd_bits+1)))) {
					vaddr -= slot->diff0;
				} else {
					vaddr -= slot->diff;
				}
				info->vaddr = vaddr;
			}
		}
	} else if(vaddr > CPU_USER_VADDR_END) {
		// Address in Proc's address space
		return -1;
	}

	return 0;
}


int
cpu_vmm_mcreate(PROCESS *prp) {
	void				*vaddr;
	unsigned			pdir_size;
	unsigned			pdir_user_boundry;
	pxe_t				*pdep;
	int					r;
	ADDRESS				*adp;
	struct pa_quantum	*pq;
	size_t				size;
	paddr_t				paddr;

	if(pae_enabled) {
		pdir_size = __PAGESIZE*4;
		pdir_user_boundry = (SYSADDR_BASE >> PD2_BITS) * sizeof(uint64_t);
	} else {
		pdir_size = __PAGESIZE*1;
		pdir_user_boundry = (SYSADDR_BASE >> PD1_BITS) * sizeof(uint32_t);
	}

	if(prp->pid == PROCMGR_PID) {
		vaddr = dummy_aspace.pgdir;
		paddr = dummy_aspace.ptroot_paddr;
	} else {
		memsize_t  resv = 0;
		part_id_t mpid = mempart_getid(prp, sys_memclass_id);
		//make sure the L1 pgtbl is < 4G because of CR3 reg limit 
		if (MEMPART_CHK_and_INCR(mpid, (memsize_t)pdir_size, &resv) != EOK) {
			return ENOMEM;
		}
		pq = pa_alloc(pdir_size, __PAGESIZE, PAQ_COLOUR_NONE, PAA_FLAG_CONTIG, NULL, restrict_4G, resv);
		if(pq == NULL) {
			MEMPART_UNDO_INCR(mpid, pdir_size, resv);
			return ENOMEM;
		}
		MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
		pq->flags |= PAQ_FLAG_SYSTEM;
		paddr = pa_quantum_to_paddr(pq);
		r = vmm_mmap(NULL, 0, pdir_size, PROT_READ|PROT_WRITE, MAP_PHYS, NULL,
					paddr, 0, 0, NOFD, &vaddr, &size, mpid);
		if(r != EOK) {
			return r;
		}
		if(pae_enabled) {
			paddr += PDPT_OFFSET;
		}
	}

	adp = prp->memory;
	adp->cpu.pgdir = vaddr;
	adp->cpu.ptroot_paddr = paddr;

	CPU_ZERO_PAGE(vaddr, pdir_user_boundry, 0);
	#define PDIR_GLOBAL(a) (void *)((uintptr_t)(a) + pdir_user_boundry)
	memcpy(PDIR_GLOBAL(adp->cpu.pgdir), PDIR_GLOBAL(pgtbl_list->pgdir), pdir_size - pdir_user_boundry);
	if(pae_enabled) {
		unsigned	i;
		paddr_t		paddrlocal = adp->cpu.ptroot_paddr - PDPT_OFFSET;

		pdep = PXE_ADD(adp->cpu.pgdir, PDPT_OFFSET);
		for(i = 0; i < 8; ++i) {
			pdep->pxe64 = (pdep->pxe64 & (__PAGESIZE - 1)) | (paddrlocal + (i & 0x3) * __PAGESIZE);
			pdep = PXE_ADD(pdep, sizeof(pdep->pxe64));
		}
	} else {
		pdep = PXE_ADD(adp->cpu.pgdir, 0xffc);
		pdep->pxe32 = (pdep->pxe32 & ( __PAGESIZE - 1)) | adp->cpu.ptroot_paddr;
	}

	INTR_LOCK(&mem_spin);
	if(prp->pid == PROCMGR_PID) {
		//Unhook from the dummy_aspace variable
		pgtbl_list = NULL;
	}
	adp->cpu.prev = &pgtbl_list;
	if((adp->cpu.next = pgtbl_list)) {
		pgtbl_list->prev = &adp->cpu.next;
	}
	pgtbl_list = &adp->cpu;
	INTR_UNLOCK(&mem_spin);

	return EOK;
}


void
cpu_vmm_mdestroy(PROCESS *prp) {
	ADDRESS				*adp;
	struct pa_quantum	*curr;
	struct pa_quantum	*next;
	part_id_t		mpid = mempart_getid(prp, sys_memclass_id);
	memsize_t			memclass_pid_free = 0;

	adp = prp->memory;
	INTR_LOCK(&mem_spin);
	if((*adp->cpu.prev = adp->cpu.next)) {
		adp->cpu.next->prev = adp->cpu.prev;
	}
	INTR_UNLOCK(&mem_spin);

	curr = adp->cpu.l2_list;
	while(curr != NULL) {
		next = curr->u.inuse.next;
		pa_free(curr, 1, MEMPART_DECR(mpid, NQUANTUM_TO_LEN(1)));
		memclass_pid_free += NQUANTUM_TO_LEN(1);
		curr = next;
	}
	MEMCLASS_PID_FREE(prp, mempart_get_classid(mpid), memclass_pid_free);
	flushtlb();
	(void) vmm_munmap(NULL, (uintptr_t)adp->cpu.pgdir, pae_enabled ? __PAGESIZE*4 : __PAGESIZE, 0, mpid);
}


// For cpu_pte_split/manipulate/merge, we're going to use the
// pa_quantum.u.inuse.qpos field to hold the starting vaddr for an L2 table.
// We're also going to sort the linked list of of L2's in increasing order
// of that vaddr. That way the split code can easily find the same L2 table
// that we were using before when it needs to re-establish the two level 
// mapping and not have to bother initializing a new one.

int
cpu_pte_split(uintptr_t vaddr, struct mm_pte_manipulate *data) {
	pxe_t				*pdep;
	uintptr_t			pde_size;
	uint64_t			pde;
	uint64_t			new_pte;
	pxe_t				*pgdir;
	ADDRESS				*adp;
	struct pa_quantum	*pq;
	paddr_t				l2_paddr;
	pxe_t				*p;
	uintptr_t			check;

	if(!(__cpu_flags & X86_CPU_PSE)) return EOK;
	if(vaddr != data->start) {
		// We took care of everything with the first call
		return EOK;
	}

	adp = data->adp;
	if(adp != NULL) {
		pgdir = adp->cpu.pgdir;
		pq = adp->cpu.l2_list;
	} else {
		pgdir = pgtbl_list->pgdir;
		pq = sys_l2_list;
	}
	pde_size = 1 << pd_bits;
	check = ROUNDDOWN(vaddr, pde_size);
	while(check < data->end) {
		pdep = GENERIC_VTOPDIRP(pgdir, check);
		pde = PXE_GET(pdep);

		if(pde & X86_PDE_PS) {
			// Have to split the the big page into individual PTE entries.
			new_pte = pde & ~X86_PDE_PS;

//kprintf("splitting big page at 0x%x\n", check);

			// Find the L2 table to use
			for( ;; ) {
				CRASHCHECK(pq == NULL);
				if(pq->u.inuse.qpos == check) break;
				pq = pq->u.inuse.next;
			}

			l2_paddr = pa_quantum_to_paddr(pq);

			// Have to zero the PDE before invalidating the vaddr so that
			// all CPU's in an SMP system will pick up the new value
			if(check <= CPU_USER_VADDR_END) {
				PXE_SET(pdep, 0);
				invlpg(check);
				SMP_FLUSH_TLB();
				PXE_SET(pdep, l2_paddr | (X86_PTE_WRITE | X86_PTE_PRESENT | X86_PTE_USER));
			} else {
				struct cpu_mm_aspace 	*dir;
				uintptr_t				pde_offset;

				pde_offset = (uintptr_t)pdep - (uintptr_t)pgdir;
				//FUTURE: Careful if we start doing PTE manipulations
				//FUTURE: while not in the kernel - a process could
				//FUTURE: could be created/destroyed while we're running
				//FUTURE: this list.
				for(dir = pgtbl_list; dir; dir = dir->next) {
					p = PXE_ADD(dir->pgdir, pde_offset);
					PXE_SET(p, 0);
				}
				invlpg(check);
				SMP_FLUSH_TLB();
				for(dir = pgtbl_list; dir; dir = dir->next) {
					p = PXE_ADD(dir->pgdir, pde_offset);
					PXE_SET(p, l2_paddr | (X86_PTE_WRITE | X86_PTE_PRESENT));
				}
			}

			if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
				data->start = check;
				return EINTR;
			}
		}
		check += pde_size;
	}
	if((check-1) > data->split_end) data->split_end = check - 1;
	
	return EOK;
}


#define PTE_CHECK_BITS (X86_PTE_PRESENT	\
					|X86_PTE_WRITE		\
					|X86_PTE_USER		\
					|X86_PTE_WT			\
					|X86_PTE_CD			\
					|X86_PTE_PAT		\
					|X86_PTE_GLOBAL		\
					|X86_PTE_USER1		\
					|X86_PTE_USER2		\
					|X86_PTE_USER3		\
					|X86_PTE_NX			\
					|PTE_PADDR_BITS)


int
cpu_pte_merge(struct mm_pte_manipulate *data) {
	uintptr_t			pde_size;
	uintptr_t			check;
	ADDRESS				*adp;
	pxe_t				*pgdir;
	pxe_t				*pdep;
	pxe_t				*kmap_pde;
	pxe_t				*ptep;
	pxe_t				*end;
	pxe_t				*try;
	uint64_t			pde;
	uint64_t			prev_pte;
	uint64_t			curr_pte;
	paddr_t				l2_paddr;
	unsigned			pxe_size;
	struct pa_quantum	*pq;

	if(!(__cpu_flags & X86_CPU_PSE)) {
		return EOK;
	}
	pde_size = 1 << pd_bits;
	if(!(data->op & PTE_OP_MERGESTARTED)) {
		data->start = ROUNDDOWN(data->start, pde_size);
		data->op |= PTE_OP_MERGESTARTED;
	}
	adp = data->adp;
	if(adp != NULL) {
		pgdir = adp->cpu.pgdir;
	} else {
		pgdir = pgtbl_list->pgdir;
	}
	check = data->start;
	while(check < data->split_end) {
		pdep = GENERIC_VTOPDIRP(pgdir, check);
		pde = PXE_GET(pdep);
		l2_paddr = pde & PTE_PADDR_BITS;
		if((pde & X86_PTE_PRESENT) && !(pde & X86_PDE_PS)) {
			// Get addressability to the L2 table
			if((adp == NULL) 
			  ||(check > CPU_USER_VADDR_END) 
			  ||(adp->cpu.ptroot_paddr == rdpgdir())) {

				// We can use the currently active page table 
				// to check the PTEs - much faster
				ptep = VTOPTP(check);
				kmap_pde = NULL;
			} else if(l2_paddr <= CPU_SYSTEM_PADDR_END) {
				ptep = (void *)((uintptr_t)l2_paddr + CPU_1TO1_VADDR_BIAS);
				kmap_pde = NULL;
			} else {
				kmap_pde = VTOPDIRP(L2MAP_BASE);
				ptep = VTOPTP(L2MAP_BASE);
				invlpg((uintptr_t)kmap_pde);
				invlpg((uintptr_t)ptep);
				PXE_SET(kmap_pde, l2_paddr | X86_PTE_PRESENT);
			}
			pxe_size = 1 << pxe_bits;
			end = (void *)((uintptr_t)ptep + __PAGESIZE);
			try = ptep;
			prev_pte = PXE_GET(try);
			if(!(prev_pte & X86_PTE_PAT) && ((prev_pte & PTE_PADDR_BITS & (pde_size - 1)) == 0)) {
				prev_pte += __PAGESIZE;
				for( ;; ) {
					try = PXE_ADD(try, pxe_size);
					if(try >= end) {
//kprintf("merging 0x%x into 0x%x pagesize\n", check, pde_size);
						// We can merge into a big page
						curr_pte = PXE_GET(ptep) | X86_PDE_PS;
						if(check <= CPU_USER_VADDR_END) {
							PXE_SET(pdep, curr_pte);
						} else {
							struct cpu_mm_aspace 	*dir;
							uintptr_t				pde_offset;

							pde_offset = (uintptr_t)pdep - (uintptr_t)pgdir;
							//FUTURE: Careful if we start doing PTE manipulations
							//FUTURE: while not in the kernel - a process could
							//FUTURE: could be created/destroyed while we're running
							//FUTURE: this list.
							for(dir = pgtbl_list; dir; dir = dir->next) {
								pxe_t			*p;

								p = PXE_ADD(dir->pgdir, pde_offset);
								PXE_SET(p, curr_pte);
							}
						}
						flushtlb();
						SMP_FLUSH_TLB();
						pq = pa_paddr_to_quantum(l2_paddr);
						CRASHCHECK(pq == NULL);
						break;
					}
					curr_pte = PXE_GET(try);
					if((prev_pte & PTE_CHECK_BITS) != (curr_pte & PTE_CHECK_BITS)) break;
					prev_pte = curr_pte + __PAGESIZE;
				}
			}
			if(kmap_pde != NULL) {
				PXE_SET(kmap_pde, 0);
			}
		}
		check += pde_size;
		if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			data->start = check;
			return EINTR;
		}
	}
	return EOK;
}


int 
cpu_pte_manipulate(struct mm_pte_manipulate *data) {
	pxe_t				*ptep;
	pxe_t				*pdep;
	pxe_t				*prev_pdep;
	pxe_t				*kmap_pde;
	pxe_t				*kmap_ptp;
	pxe_t				*l2_vaddr;
	pxe_t				*pgdir;
	uint64_t			pte;
	uint64_t			orig_pte;
	paddr_t				l2_paddr;
	uint64_t			bits;
	unsigned			cr0;
	struct pa_quantum	*pq;
	struct pa_quantum	**l2_owner;
	unsigned			pa_status;
	unsigned			pde_size;
	ADDRESS				*adp;
	int					r;
	unsigned			flags;
	uintptr_t			curr;
	unsigned			op;
	part_id_t		mpid;
	PROCESS *			prp;

	#define PMF_USER		0x0001 // mapping in user address range
	#define PMF_ACTIVE		0x0002 // can use the active page table
	#define PMF_KMAP_USED	0x0004
	#define PMF_INVLPG		0x0008
	#define PMF_NEEDSFLUSH	0x0010
	#define PMF_SMPFLUSH	0x0020
	#define PMF_CR0_FLIPPED 0x0040

	// Assume alignment has been checked by the caller

	op = data->op;
	if(op & PTE_OP_BAD) {
		// Fault code uses this as a trigger to immediately fail the
		// page access
		bits = X86_PTE_USER2;
	} else {
		// X86_PTE_USER1 indicates that we've actually mapped the page
		bits = X86_PTE_USER1;
		if(data->prot & (PROT_READ|PROT_EXEC|PROT_WRITE)) bits |= X86_PTE_PRESENT;
		if(data->prot & PROT_WRITE) bits |= X86_PTE_WRITE;
		if(data->prot & PROT_NOCACHE) bits |= X86_PTE_CD | X86_PTE_WT;
		if(!(data->prot & PROT_EXEC) && pae_enabled && (__cpu_flags & X86_CPU_NX)) {
			bits |= X86_PTE_NX;
		}
		//RUSH3: Turn on X86_PTE_GLOBAL if in system space?
		//RUSH3: If we do that, the SMP_FLUSH_TLB won't work, since 
		//RUSH3: it doesn't flush global entries.
	}
	adp = data->adp;
	if(adp != NULL) {
		pgdir = adp->cpu.pgdir;
	} else {
		pgdir = pgtbl_list->pgdir;
	}
	curr = data->start;
	if(data->shmem_flags & SHMCTL_LAZYWRITE) {
		if(op & PTE_OP_MAP) {
			(void) x86_set_mtrr(data->paddr, (data->end - curr) + 1, data->shmem_flags, op);
		} else if(op & PTE_OP_UNMAP) {
			paddr_t		paddr;

			// The unmap doesn't set the paddr, so we have to get it from
			// the page table.
			if(cpu_vmm_vaddrinfo(adp ? object_from_data(adp, address_cookie) : NULL, curr, &paddr, NULL) != PROT_NONE) {
				(void) x86_set_mtrr(paddr, (data->end - curr) + 1, data->shmem_flags, op);
			}
		}
		// Don't want to do the x86_set_mttr() again if we get preempted.
		data->shmem_flags &= ~SHMCTL_LAZYWRITE;
	}
	flags = PMF_ACTIVE;
	if(WITHIN_BOUNDRY(curr, curr, user_boundry_addr)) {
		flags |= PMF_USER;
		if(adp == NULL) crash();
		if(adp->cpu.ptroot_paddr != rdpgdir()) {
			flags &= ~PMF_ACTIVE;
		}

		if(!(op & PTE_OP_TEMP)) {
			bits |= X86_PTE_USER;
		}
		l2_owner = &adp->cpu.l2_list;
	} else {
		l2_owner = &sys_l2_list;
	}
	if(__cpu_flags & X86_CPU_INVLPG) {
		flags |= PMF_INVLPG;
	}

	kmap_pde = VTOPDIRP(L2MAP_BASE);
	kmap_ptp = VTOPTP(L2MAP_BASE);
	l2_vaddr = NULL;
	prev_pdep = NULL;
	pde_size = (1 << pd_bits);

	r = EOK;
	cr0 = 0; // Dummy assignment to shut GCC up.
	pa_status = 0; //shut Lint up.
	prp = adp ? object_from_data(adp, address_cookie) : NULL;
	mpid = mempart_getid(prp, sys_memclass_id);
	for( ;; ) {
		if(curr >= data->end) break;

		pdep = GENERIC_VTOPDIRP(pgdir, curr);
		if(pdep != prev_pdep) {
			// Switching to new L2 page table.
			prev_pdep = pdep;
			if(PXE_GET_FLAGS(pdep) & X86_PTE_PRESENT) {
				l2_paddr = PXE_GET(pdep) & PTE_PADDR_BITS;
				pq = NULL;
			} else {
				memsize_t  resv = 0;

				if(!(op & (PTE_OP_MAP|PTE_OP_PREALLOC|PTE_OP_BAD))) {
					//Move vaddr to next page directory
					curr = (curr + pde_size) & ~(pde_size - 1);
					continue;
				}
			
				// Find the spot in the list for the new L2
				for( ;; ) {
					pq = *l2_owner;
					if(pq == NULL) break;
					if(pq->u.inuse.qpos > curr) break;
					l2_owner = &pq->u.inuse.next;
				}

				// Allocate a new L2
				if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
					r = ENOMEM;
					break;
				}
				pq = pa_alloc(__PAGESIZE, __PAGESIZE, PAQ_COLOUR_NONE, 0, &pa_status, restrict_proc, resv);
				if(pq == NULL) {
					MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
					r = ENOMEM;
					break;
				}
				MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
				pq->flags |= PAQ_FLAG_SYSTEM;
				pq->u.inuse.qpos = ROUNDDOWN(curr,pde_size);
				pq->u.inuse.next = *l2_owner;
				*l2_owner = pq;

				l2_paddr = pa_quantum_to_paddr(pq);
			}

			// Get addressability to the L2 page table
			if(CPU_1TO1_IS_PADDR(l2_paddr)) {
				// Use the 1-to-1 area to get addressability.
				l2_vaddr = (void *)((uintptr_t)l2_paddr + CPU_1TO1_VADDR_BIAS);
			} else {
				if(!(flags & PMF_CR0_FLIPPED)) {
					flags |= PMF_CR0_FLIPPED;
					cr0 = rdcr0();
					ldcr0(cr0 & ~X86_MSW_WP_BIT);
				}
				if((flags & PMF_ACTIVE) && (pq == NULL)) {
					// An active, already existing L2.
					l2_vaddr = VTOPTP(curr);
				} else {
					// Darn, have to set up a mapping to get to the L2 :-(.
					if(flags & PMF_INVLPG) {
						invlpg((uintptr_t)kmap_pde);
						invlpg((uintptr_t)kmap_ptp);
					} else {
						flushtlb();
						flags &= ~PMF_NEEDSFLUSH;
					}
					PXE_SET(kmap_pde, l2_paddr | (X86_PTE_WRITE|X86_PTE_PRESENT));
					l2_vaddr = kmap_ptp;
					flags |= PMF_KMAP_USED;
				}
			}

			if(pq != NULL) {
				// Have a newly allocated page table, put it in..

				// Zero the page directory first (think SMP)
				if(pa_status & PAA_STATUS_NOT_ZEROED) {
					CPU_ZERO_PAGE(l2_vaddr, __PAGESIZE, 0);
				}

				if(flags & PMF_USER) {
					PXE_SET(pdep, l2_paddr | (X86_PTE_WRITE | X86_PTE_PRESENT | X86_PTE_USER));
				} else {
					struct cpu_mm_aspace 	*dir;
					uintptr_t				pde_offset;

					pde_offset = (uintptr_t)pdep - (uintptr_t)pgdir;
					//FUTURE: Careful if we start doing PTE manipulations
					//FUTURE: while not in the kernel - a process could
					//FUTURE: could be created/destroyed while we're running
					//FUTURE: this list.
					for(dir = pgtbl_list; dir; dir = dir->next) {
						pxe_t			*p;

						p = PXE_ADD(dir->pgdir, pde_offset);
						PXE_SET(p, l2_paddr | (X86_PTE_WRITE | X86_PTE_PRESENT));
					}
				}
			}
			if(op & PTE_OP_PREALLOC) {
				//Move vaddr to next page directory
				curr = (curr + pde_size) & ~(pde_size - 1);
				continue;
			}
		}
		ptep = (pxe_t *)((uintptr_t)l2_vaddr + (((curr & (pde_size-1)) >> 12) << pxe_bits));
		orig_pte = PXE_GET(ptep);
		if(op & (PTE_OP_MAP|PTE_OP_BAD)) {
			pte = data->paddr | bits;
		} else if(op & PTE_OP_UNMAP) {
			pte = 0;
		} else if(orig_pte & X86_PTE_USER1) {
			// PTE_OP_PROT
			pte = (orig_pte & (PTE_PADDR_BITS|X86_PTE_ACCESSED|X86_PTE_DIRTY)) 
					| bits;
		} else {
			// We don't change PTE permissions if we haven't mapped the
			// page yet...
			pte = orig_pte;
		}

		PXE_SET(ptep, pte);
		if(pte != orig_pte && (orig_pte & X86_PTE_PRESENT)) {
			/*
			 * we must make sure that on a page fault, the invalid TLB
			 * entry is flushed so the NOT present value invalidated. Then
			 * this optimization will work.
			 */
			if(flags & PMF_INVLPG) {
				invlpg(curr);
			} else {
				flags |= PMF_NEEDSFLUSH;
			}
#if 0
//RUSH2: Before this can be enabled, we have to make sure that
//RUSH2: the cpu_vmm_fault() code invalidates any page that comes to
//RUSH2: remove it from the TLB. Think about propagating this change
//RUSH2: to PPC and MIPS as well.
#define PERM_BITS (X86_PTE_PRESENT|X86_PTE_WRITE|X86_PTE_NX)				
			if((pte & ~PERM_BITS) != (orig_pte & ~PERM_BITS)) {
				flags |= PMF_SMPFLUSH;
			} else {
				uint64_t	old_perms = orig_pte & PERM_BITS;
				uint64_t	new_perms = pte & PERM_BITS;

				if((old_perms ^ new_perms) & old_perms) {
					// The new perm bits has something turned off
					// from the old ones.
					flags |= PMF_SMPFLUSH;
				}
			}
#else
			flags |= PMF_SMPFLUSH;
#endif
		}
		curr += __PAGESIZE;
		data->paddr += __PAGESIZE;
		if((op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			r = EINTR;
			break;
		}
	}

	if(flags & PMF_CR0_FLIPPED) {
		if(flags & PMF_KMAP_USED) {
			PXE_SET(kmap_pde, 0);
		}
		ldcr0(cr0);
	}
	if(flags & PMF_NEEDSFLUSH) {
		flushtlb();
	}
	if(flags & PMF_SMPFLUSH) {
		SMP_FLUSH_TLB();
	}
	data->start = curr;
	return r;
}

	
unsigned 
cpu_vmm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *paddrp, size_t *lenp) {
	uint64_t	pde;
	pxe_t		*pgdir;
	pxe_t		*kmap_pde;
	pxe_t		*kmap_ptp;
	pxe_t		*ptep;
	uint64_t	pte;
	unsigned	cr0;
	int			inval;
	unsigned	pde_mask = (1 << pd_bits) - 1;
	unsigned	pg_offset;
	unsigned 	prev_flags;
	unsigned	prot;

	if(prp != NULL && prp->memory != NULL) {
		pgdir = prp->memory->cpu.pgdir;
	} else {
		pgdir = pgtbl_list->pgdir;
	}
	pde = PXE_GET(GENERIC_VTOPDIRP(pgdir, vaddr));

	if(!(pde & X86_PTE_PRESENT)) return PROT_NONE;

	if(pde & X86_PDE_PS) {
		if(!(pde & (X86_PDE_USER1|X86_PDE_PRESENT))) return PROT_NONE;
		pg_offset = vaddr & pde_mask;
		*paddrp = (pde & ~(X86_PTE_NX | pde_mask)) + pg_offset;
		if(lenp != NULL) *lenp = (pde_mask + 1) - pg_offset;
		prot = MAP_PHYS; // So we don't return PROT_NONE
		if(pde & X86_PDE_PRESENT) {
			prot |= PROT_READ;
			if(pde & X86_PDE_WRITE) prot |= PROT_WRITE;
			if(!pae_enabled || !(__cpu_flags & X86_CPU_NX) || !(pde & X86_PDE_NX)) {
				prot |= PROT_EXEC;
			}
		}
		return prot;
	}

	if((prp == NULL) 
	  ||(vaddr > CPU_USER_VADDR_END) 
	  ||(prp->memory->cpu.ptroot_paddr == rdpgdir())) {

		// We can use the currently active page table 
		// to check the PTE - much faster
		ptep = VTOPTEP(vaddr);
		pte = PXE_GET(ptep);
	} else {
		//RUSH3: Use 1-to-1 mapping area if possible
		inval = __cpu_flags & X86_CPU_INVLPG;
		cr0 = rdcr0();
		kmap_pde = VTOPDIRP(L2MAP_BASE);
		kmap_ptp = VTOPTP(L2MAP_BASE);

		//RUSH3: The disable()/restore() sequence probably only needs to
		//RUSH3: be around the ldpgdir(rdpgdir()) sequence now.
		prev_flags = disable();
		if(inval) {
			invlpg((unsigned)kmap_pde);
			invlpg((unsigned)kmap_ptp);
		} else {
			// Can't use flushtlb() here because it re-enables interrupts.
			// This sequence is OK because interrupts are already disabled
			// and will be re-enabled (if required) by the restore() call
			// below.
			ldpgdir(rdpgdir());
		}
		ldcr0(cr0 & ~X86_MSW_WP_BIT);
		PXE_SET(kmap_pde, (pde & PTE_PADDR_BITS) | X86_PTE_PRESENT);
		ptep = (pxe_t *)((uintptr_t)kmap_ptp + (((vaddr & pde_mask) >> 12) << pxe_bits));
		pte = PXE_GET(ptep);
		PXE_SET(kmap_pde, 0);
		ldcr0(cr0);
		restore(prev_flags);
	}
	if(!(pte & (X86_PTE_USER1|X86_PTE_PRESENT))) return PROT_NONE;
	pg_offset = ADDR_OFFSET(vaddr);
	*paddrp = (pte & PTE_PADDR_BITS) | pg_offset;
	if(lenp != NULL) *lenp = __PAGESIZE - pg_offset;
	prot = MAP_PHYS; // So we don't return PROT_NONE
	if(pte & X86_PTE_PRESENT) {
		prot |= PROT_READ;
		if(pte & X86_PTE_WRITE) prot |= PROT_WRITE;
		if(!pae_enabled || !(__cpu_flags & X86_CPU_NX) || !(pte & X86_PTE_NX)) {
			prot |= PROT_EXEC;
		}
	}
	return prot;
}

__SRCVERSION("cpu_vmm.c $Rev: 175229 $");
