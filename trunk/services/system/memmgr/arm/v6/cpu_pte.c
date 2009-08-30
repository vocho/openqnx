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

#ifdef	VARIANT_smp

void
smp_ptp_set(ptp_t *ptp, unsigned val)
{
	int	i;

	/*
	 * ptp is a pointer in the cpu0 L1 table.
	 * By adding 16K to this pointer, we can move through each cpu's L1 table.
	 */
	for (i = 0; i < NUM_PROCESSORS; i++) {
		*ptp = val;
		ptp += (ARM_L1_SIZE / sizeof(ptp_t));
	}
}

void
smp_pde_set(pte_t *pde, unsigned val)
{
	int			i;
	unsigned	off;

	if (NUM_PROCESSORS == 1) {
		/*
		 * L2_vaddr is not set for 1-cpu systems
		 */
		*pde = val;
		return;
	}

	off = pde - (pte_t *)ARM_PTP_BASE;
	/*
	 * pde is now a pointer in the cpu0 L2 table.
	 * By adding 4K to this pointer, we can move through each cpu's L2 table.
	 */
	pde = L2_vaddr + off;
	for (i = 0; i < NUM_PROCESSORS; i++) {
		*pde = val;
		pde += __PAGESIZE / sizeof *pde;
	}
}
#else
#define	smp_ptp_set(ptp, val)	(*(ptp) = (val))
#define	smp_pde_set(pde, val)	(*(pde) = (val))
#endif

/*
 * Zero a 4K page table by mapping it at ARM_V6_SCRATCH_PTBL
 *
 * This translation is made only on RUNCPU
 */
void
ptzero(struct pa_quantum *qp)
{
	unsigned	colour;
	paddr_t		paddr;
	unsigned	vaddr;
	pte_t		*pte;

	paddr = pa_quantum_to_paddr(qp);

	colour = PAQ_GET_COLOUR(qp);
	if (colour == PAQ_COLOUR_NONE) {
		colour = COLOUR_PA(paddr);
	}
	vaddr = ARM_V6_COLOUR_BASE + (colour << ADDR_OFFSET_BITS);
	pte = KTOPTEP(vaddr);
	*pte = paddr | arm_cpu->kpte_rw;
	CPU_ZERO_PAGE((void *)vaddr, __PAGESIZE, 0);
	CacheControl((void *)vaddr, __PAGESIZE, MS_INVALIDATE);
	*pte = 0;
	arm_v4_dtlb_addr(vaddr);	// global TLB entry - no asid required
}

/*
 * Map page table for user address space.
 */
static void
ptmap_user(unsigned va, paddr_t pa, int inactive)
{
	pte_t				*pte;
	ptp_t				*ptp;

	if (inactive) {
		/*
		 * Just manipulate the L1 entries
		 */
		ptp = ITOL1PT(va);
	}
	else {
		/*
		 * Map page table into the "page directory"
		 *
		 * The pte entry is marked non-global so that the mapping in the
		 * ARM_UPTE_BASE region does not require a TLB flush on context switches.
		 */
		pte = UTOPDIR(va);
		*pte = pa | l2_prot | ARM_PTE_V6_nG;

		ptp = UTOL1PT(va);
	}

	/*
	 * Create L1 entries
	 */
	pa |= ARM_PTP_V6_L2;
	*ptp++ = pa; pa += ARM_L2_SIZE;
	*ptp++ = pa; pa += ARM_L2_SIZE;
	*ptp++ = pa; pa += ARM_L2_SIZE;
	*ptp   = pa;
	arm_v6_dsb();
}

/*
 * Map page table into system address space.
 * The mapping is propagated to all cpus' L1 and page directory tables.
 */
static void
ptmap_kern(unsigned va, paddr_t pa)
{
	pte_t				*pte;
	ptp_t				*ptp;

	/*
	 * Map page table into the "page directory"
	 */
	pte = KTOPDIR(va);
	smp_pde_set(pte, pa | l2_prot);

	/*
	 * Create L1 entries
	 */
	ptp = KTOL1PT(va);
	pa |= ARM_PTP_V6_L2;
	smp_ptp_set(ptp++, pa); pa += ARM_L2_SIZE;
	smp_ptp_set(ptp++, pa); pa += ARM_L2_SIZE;
	smp_ptp_set(ptp++, pa); pa += ARM_L2_SIZE;
	smp_ptp_set(ptp, pa);
	arm_v6_dsb();
}

/*
 * Map/unmap L1 table for inactive address space
 *
 * These translations are made only RUNCPU
 */
static inline void
inactive_l1_map(ADDRESS *adp)
{
	ptp_t	*l1 = KTOPTEP(ARM_V6_INACTIVE_L1);

	l1[0] = adp->cpu.l1_pte & ~ARM_PTE_V6_nG;
	l1[1] = l1[0] + __PAGESIZE;
	arm_v4_dtlb_addr(ARM_V6_INACTIVE_L1);				// global TLB entry - no asid required
	arm_v4_dtlb_addr(ARM_V6_INACTIVE_L1 + __PAGESIZE);	// global TLB entry - no asid required
}

static inline void
inactive_l1_unmap()
{
	ptp_t	*l1 = KTOPTEP(ARM_V6_INACTIVE_L1);

	l1[0] = 0;
	l1[1] = 0;
	arm_v4_dtlb_addr(ARM_V6_INACTIVE_L1);				// global TLB entry - no asid required
	arm_v4_dtlb_addr(ARM_V6_INACTIVE_L1 + __PAGESIZE);	// global TLB entry - no asid required
}

/*
 * Map/unmap an L2 table for an inactive address space
 *
 * These translations are made only RUNCPU
 */
static inline void
inactive_l2_map(ptp_t ptp)
{
	*KTOPTEP(ARM_V6_INACTIVE_L2) = (ptp & ~PGMASK) | l2_prot;
	arm_v4_dtlb_addr(ARM_V6_INACTIVE_L2);	// global TLB entry - no asid required
}

static inline void
inactive_l2_unmap()
{
	*KTOPTEP(ARM_V6_INACTIVE_L2) = 0;
	arm_v4_dtlb_addr(ARM_V6_INACTIVE_L2);	// global TLB entry - no asid required
}

/*
 * Map/unmap page directory for an inactive address space
 *
 * These translations are made only RUNCPU
 */
static inline void
inactive_pd_map(ADDRESS *adp)
{
	*KTOPTEP(ARM_V6_SCRATCH_PTBL) = adp->cpu.l2_pte & ~ARM_PTE_V6_nG;
	arm_v4_dtlb_addr(ARM_V6_SCRATCH_PTBL);	// global TLB entry - no asid required
}

static inline void
inactive_pd_unmap()
{
	*KTOPTEP(ARM_V6_SCRATCH_PTBL) = 0;
	arm_v4_dtlb_addr(ARM_V6_SCRATCH_PTBL);	// global TLB entry - no asid required
}

/*
 * Convert a large page L2 descriptor to small page L2 descriptor
 */
static inline unsigned
lp_to_sp(pte_t opte)
{
	unsigned	npte;

	npte  = opte & ~ARM_PTE_VALID;
	npte |= ARM_PTE_SP;								// set small page

	npte &= ~ARM_PTE_V6_LP_TEX_MASK;				// update TEX bits
	npte |= (opte & ARM_PTE_V6_LP_TEX_MASK) >> 6;

	if (opte & ARM_PTE_V6_LP_XN) {					// update XN bit
		npte &= ~ARM_PTE_V6_LP_XN;
		npte |= ARM_PTE_V6_SP_XN;
	}
	return npte;
}

/*
 * Convert a small page L2 descriptor to large page L2 descriptor
 */
static inline unsigned
sp_to_lp(pte_t opte)
{
	unsigned	npte;

	npte  = opte &~ARM_PTE_VALID;
	npte |= ARM_PTE_LP;								// set large page

	npte &= ~0xf000;								// clear paddr 15:12

	npte &= ~ARM_PTE_V6_SP_TEX_MASK;				// update TEX bits
	npte |= (opte & ARM_PTE_V6_SP_TEX_MASK) << 6;

	if (opte & ARM_PTE_V6_SP_XN) {					// update XN bit
		npte &= ~ARM_PTE_V6_SP_XN;
		npte |= ARM_PTE_V6_LP_XN;
	}
	return npte;
}

/*
 * Convert L2 descriptor into L1 section descriptor
 */
static inline unsigned
l2_to_sc(pte_t opte)
{
	unsigned	ptp;
#define	ACC_BITS		\
	(ARM_PTE_V6_nG|ARM_PTE_V6_S|ARM_PTE_V6_APX|ARM_PTE_V6_AP1|ARM_PTE_V6_AP0)

	ptp  = opte & ~ARM_SCMASK;		// paddr
	ptp |= ARM_PTP_V6_SC;
	ptp |= opte & ARM_PTE_CB;
	ptp |= (opte & ACC_BITS) << 6;
	if ((opte & ARM_PTE_VALID) == ARM_PTE_LP) {
		ptp |= opte & ARM_PTE_V6_LP_TEX_MASK;
		if (opte & ARM_PTE_V6_LP_XN) {
			ptp |= ARM_PTP_V6_XN;
		}
	}
	else {
		ptp |= (opte & ARM_PTE_V6_SP_TEX_MASK) << 6;
		if (opte & ARM_PTE_V6_SP_XN) {
			ptp |= ARM_PTP_V6_XN;
		}
	}
	return ptp;
}

/*
 * Flags used in cpu_pte_split/merge/manipulate
 */
#define	PMF_FLUSH_DEFER		(1<<0)
#define	PMF_FLUSH_CACHE		(1<<1)	
#define	PMF_INACTIVE		(1<<2)
#define	PMF_USER_SPACE		(1<<3)
#define	PMF_FLUSH_SMP		(1<<4)

int
cpu_pte_split(uintptr_t vaddr, struct mm_pte_manipulate *data)
{
	int			flags = 0;
	int			r = EOK;
	unsigned	va;
	unsigned	asid = 0;

	if (data->op & PTE_OP_MERGESTARTED) {
		/*
		 * We took care of everything with the first call
		 */
		return EOK;
	}

	/*
	 * Check what address space we are manipulating
	 */
	if (V6_USER_SPACE(vaddr)) {
		ADDRESS		*adp = data->adp;
		unsigned	ttbr0 = arm_v6_ttbr0_get();

		asid = adp->cpu.asid;
		if ((ttbr0 & ~TTBR0_MASK) != (adp->cpu.ttbr0 & ~TTBR0_MASK)) {
			flags |= PMF_INACTIVE;
			inactive_l1_map(adp);
			inactive_pd_map(adp);
			arm_v6_dsb();
		}
		flags |= PMF_USER_SPACE;
	}

	/*
	 * Work on 1MB boundaries to break up section mappings if necessary
	 */
	va = ROUNDDOWN(vaddr, ARM_SCSIZE);
	while (va < data->end) {
		pte_t		*pte;
		ptp_t		*ptp;

		/*
		 * Check if anything is mapped in this area
		 */
		if (flags & PMF_USER_SPACE) {
			if (flags & PMF_INACTIVE) {
				ptp = ITOL1SC(va);
				pte = ITOPDIR(va);
			}
			else {
				ptp = UTOL1SC(va);
				pte = UTOPDIR(va);
			}
		}
		else {
			ptp = KTOL1SC(va);
			pte = KTOPDIR(va);
		}
		if ((*pte & ARM_PTE_VALID) == 0) {
			/*
			 * No page tables covering this area
			 */
			va += ARM_SCSIZE;
		}
		else {
			/*
			 * Check if we need to split a section mapping
			 */
			if ((*ptp & ARM_PTP_VALID) == ARM_PTP_V6_SC) {
				unsigned	l2_addr;

				/*
				 * Set L1 descriptor back to a L2 page table pointer
				 */
				l2_addr  = (*pte & ~PGMASK);
				l2_addr += ((va >> 20) & 3) * ARM_L2_SIZE;
				l2_addr |= ARM_PTP_V6_L2;

				*ptp = l2_addr;		
				arm_v4_idtlb_addr(va | asid);
				SMP_FLUSH_TLB();			// FIXME_smp

				/*
				 * Update data->split_end so cpu_pte_merge() can attempt to
				 * merge mappings in this 1MB region beyond data->end
				 */
				data->split_end = va + ARM_SCSIZE - 1;
			}
			CRASHCHECK((*ptp & ARM_PTP_VALID) != ARM_PTP_V6_L2);

			/*
			 * Check if we need to split large page mappings
			 */
			va  = ROUNDDOWN(vaddr, ARM_LPSIZE);
			if (flags & PMF_INACTIVE) {
				pte = ITOPTEP(va);
				inactive_l2_map(*ptp);
				arm_v6_dsb();
			}
			else {
				pte = (flags & PMF_USER_SPACE) ? UTOPTEP(va) : KTOPTEP(va);
			}
			do {
				if ((*pte & ARM_PTE_VALID) == ARM_PTE_LP) {
					unsigned	npte  = lp_to_sp(*pte);
					int			i;

					for (i = 0; i < 16; i++) {
						pte[i] = npte;
						npte += __PAGESIZE;
					}
					arm_v4_idtlb_addr(va | asid);
					SMP_FLUSH_TLB();			// FIXME_smp
				}
				va  += ARM_LPSIZE;
				pte += 16; 
			} while (va < data->end && (va & ARM_SCMASK) != 0);
		}

		/*
		 * va is now at next 1MB boundary or 64K boundary following data->end
		 */
		vaddr = va;
		if (vaddr > data->split_end) {
			data->split_end = vaddr - 1;
		}
		if ((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			data->start = vaddr;
			r = EINTR;
			break;
		}
	}

	if (flags & PMF_INACTIVE) {
		inactive_l1_unmap();
		inactive_l2_unmap();
		inactive_pd_unmap();
		arm_v6_dsb();
	}

	if (r == EOK) {
		/*
		 * Everything was split
		 */
		data->op |= PTE_OP_MERGESTARTED;
	}
	return r;
}

int
cpu_pte_merge(struct mm_pte_manipulate *data)
{
	int			flags = 0;
	unsigned	vaddr = data->start;
	int			r = EOK;
	unsigned	va;
	unsigned	asid = 0;

	/*
	 * Check what address space we are manipulating
	 */
	if (V6_USER_SPACE(vaddr)) {
		ADDRESS		*adp = data->adp;
		unsigned	ttbr0 = arm_v6_ttbr0_get();

		asid = adp->cpu.asid;
		if ((ttbr0 & ~TTBR0_MASK) != (adp->cpu.ttbr0 & ~TTBR0_MASK)) {
			flags |= PMF_INACTIVE;
			inactive_l1_map(adp);
			inactive_pd_map(adp);
			arm_v6_dsb();
		}
		flags |= PMF_USER_SPACE;
	}

	/*
	 * Work on 1MB boundaries to allow merging sections if possible
	 */
	va = ROUNDDOWN(vaddr, ARM_SCSIZE);
	while (va < data->split_end) {
		pte_t		*pte;
		ptp_t		*ptp;

		if (flags & PMF_USER_SPACE) {
			if (flags & PMF_INACTIVE) {
				ptp = ITOL1SC(va);
				pte = ITOPDIR(va);
			}
			else {
				ptp = UTOL1SC(va);
				pte = UTOPDIR(va);
			}
		}
		else {
			ptp = KTOL1SC(va);
			pte = KTOPDIR(va);
		}

		/*
		 * No need to do anything if area is unmapped or already a section
		 */
		if ((*pte & ARM_PTE_VALID) == 0 || (*ptp & ARM_PTP_VALID) == ARM_PTP_V6_SC) {
			/*
			 * Area is unmapped or already has a section mapping
			 */
			va += ARM_SCSIZE;
		}
		else {
			pte_t		opte;
			pte_t		npte;
			unsigned	size;
			int			npg;

			/*
			 * Check if we can coalesce this 1MB region into a section entry:
			 * - paddr must be 1MB aligned
			 * - all ptes must map physically contiguous addresses
			 * - all ptes must have same access protections
			 *
			 * To keep the contiguity checks simple, we convert large page
			 * descriptors to small page descriptors. This means we can
			 * simply add the size of the mapping to each entry to determine
			 * the required value of the next descriptor.
			 */
			if (flags & PMF_INACTIVE) {
				pte = ITOPTEP(va);
				inactive_l2_map(*ptp);
				arm_v6_dsb();
			}
			else {
				pte = (flags & PMF_USER_SPACE) ? UTOPTEP(va) : KTOPTEP(va);
			}
			opte = *pte;
			npg = 0;
			if ((opte & ARM_PTE_VALID) != 0) {
				if ((opte & ARM_PTE_VALID) == ARM_PTE_LP) {
					opte = lp_to_sp(opte);
					npg = 16;
				}
				else {
					npg = 1;
				}
				size = npg << 12;
				if (((opte & ~(size-1)) & ARM_SCMASK) == 0) {
					do {
						npte = opte + size;
						opte = pte[npg];
						if ((opte & ARM_PTE_VALID) == ARM_PTE_LP) {
							opte = lp_to_sp(opte);
							size = ARM_LPSIZE;
							npg += 16;
						}
						else {
							size = __PAGESIZE;
							npg++;
						}
						if (opte != npte) {
							break;
						}
					} while (npg < ARM_SCSIZE / __PAGESIZE);
				}
			}
			if (npg == ARM_SCSIZE / __PAGESIZE) {
				/*
				 * Can coalesce into a section mapping
				 */
				*ptp = l2_to_sc(*pte);
				npg = 0;
				while (npg < ARM_SCSIZE / __PAGESIZE) {
					arm_v4_idtlb_addr(va | asid);
					if ((pte[npg] & ARM_PTE_VALID) == ARM_PTE_LP) {
						va += ARM_LPSIZE;
						npg += 16;
					}
					else {
						va += __PAGESIZE;
						npg++;
					}
				}
				SMP_FLUSH_TLB();			// FIXME_smp
			}
			else {
				/*
				 * Check if we can coalesce into large pages:
				 * - paddr must be 64K aligned
				 * - all ptes must map physically contiguous addresses
				 * - all ptes must have same access protections
				 */
				va = ROUNDDOWN(vaddr, ARM_LPSIZE);
				if (flags & PMF_USER_SPACE) {
					pte = (flags & PMF_INACTIVE) ? ITOPTEP(va) : UTOPTEP(va);
				}
				else {
					pte = KTOPTEP(va);
				}
				do {
					unsigned	optelocal = *pte;

					if ((optelocal & ARM_PTE_VALID) && (optelocal & ARM_PTE_VALID) != ARM_PTE_LP && ((optelocal & ~PGMASK) & ARM_LPMASK) == 0) {
						for (npg = 1; npg < 16; npg++) {
							if (pte[npg] != optelocal + __PAGESIZE) {
								break;
							}
							optelocal = pte[npg];
						}
						if (npg == 16) {
							/*
							 * Can create large page entries
							 */
							unsigned	tva = va;

							npte = sp_to_lp(optelocal);
							for (npg = 0; npg < 16; npg++) {
								pte[npg] = npte;
								arm_v4_idtlb_addr(tva | asid);
								tva += __PAGESIZE;
							}
							SMP_FLUSH_TLB();	// FIXME_smp
						}
					}
					va += ARM_LPSIZE;
					pte += 16;
				} while (va < data->end && (va & ARM_SCMASK) != 0);
			}
		}

		/*
		 * va is now at next 1MB boundary or 64K boundary following data->end
		 */
		vaddr = va;
		if ((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			data->start = vaddr;
			r = EINTR;
			break;
		}
	}

	if (flags & PMF_INACTIVE) {
		inactive_l1_unmap();
		inactive_l2_unmap();
		inactive_pd_unmap();
		arm_v6_dsb();
	}
	return r;
}


int
cpu_pte_manipulate(struct mm_pte_manipulate *data)
{
	ADDRESS		*adp = data->adp;
	unsigned	va   = data->start;
	unsigned	end  = data->end;
	unsigned	prot;
	int			r;
	unsigned	flags = 0;

	// Assume alignment has been checked by the caller

	/*
	 * Figure out what address space we are acting on
	 */
	if (V6_USER_SPACE(va)) {
		unsigned	ttbr0 = arm_v6_ttbr0_get();

		if ((ttbr0 & ~TTBR0_MASK) != (adp->cpu.ttbr0 & ~TTBR0_MASK)) {
			flags |= PMF_INACTIVE;
			inactive_l1_map(adp);
			arm_v6_dsb();
		}
		flags |= PMF_USER_SPACE;
	}

	/*
	 * Figure out page table access protection if necessary
	 */
	prot = 0;
	if (data->op & PTE_OP_BAD) {
		/*
		 * We will create an invalid pte with a known signature that
		 * is detected by cpu_vmm_fault()
		 */
		prot = ARM_PTE_BAD;
	}
	else if (data->op & (PTE_OP_MAP|PTE_OP_PROT)) {
		/*
		 * Figure out access protections to use
		 *
		 * FIXME: PTE_OP_TEMP mappings should also be considered privileged,
		 *        but the kernel message pass code uses user privileges if
		 *        the destination is a "user" address. This will cause any
		 *        proc_read() to these temp mappings to fault.
		 *
		 *        This will create a window where a multithreaded process
		 *        may find access to this temp mapping while the page is
		 *        being initialised, instead of faulting.
		 */
		if (flags & PMF_USER_SPACE) {
			if (data->prot & PROT_WRITE) {
				prot = arm_cpu->upte_rw;
			}
			else if (data->prot & (PROT_READ|PROT_EXEC)) {
				prot = arm_cpu->upte_ro;
			}
		}
		else {
			if (data->prot & PROT_WRITE) {
				prot = arm_cpu->kpte_rw;
			}
			else if (data->prot & (PROT_READ|PROT_EXEC)) {
				prot = arm_cpu->kpte_ro;
			}
		}
		if (data->prot & PROT_NOCACHE) {
			prot &= ~arm_cpu->mask_nc;
		}
#ifdef	FIXME
		if ((data->prot & PROT_EXEC) == 0) {
			prot |= ARM_PTE_V6_SP_XN;
		}
#endif
		if (data->shmem_flags & SHMCTL_LAZYWRITE) {
			/*
			 * Implement "lazy writing" as shared device: TEX=0,C=0,B=1
			 *
			 * This provides uncached access but memory writes can be merged
			 * into write buffer to avoid the cpu stalling for normal uncached
			 * (strongly ordered TEX=0,C=0,B=1) access.
			 */
			prot &= ~arm_cpu->mask_nc;
			prot |= ARM_PTE_B;
		}
		if (data->shmem_flags & SHMCTL_HAS_SPECIAL) {
			/*
			 * Allow custom setting of S/TEX/C/B/XN bits
			 */
			#define PTE_SPECIAL_BITS \
					(ARM_PTE_V6_S | ARM_PTE_V6_SP_TEX_MASK | ARM_PTE_CB | ARM_PTE_V6_SP_XN)

			prot &= ~PTE_SPECIAL_BITS;
			prot |= data->special & PTE_SPECIAL_BITS;
		}
	}

	r = EOK;
	while (va < end) {
		pte_t				*pte;
		volatile ptp_t		*ptp;

		if (flags & PMF_USER_SPACE) {
			if (flags & PMF_INACTIVE) {
				ptp = ITOL1SC(va);
				pte = ITOPTEP(va);
			}
			else {
				ptp = UTOL1SC(va);
				pte = UTOPTEP(va);
			}
		}
		else {
			ptp = KTOL1SC(va);
			pte = KTOPTEP(va);
		}

		/*
		 * Check if we have pages tables covering this 4MB region
		 */
		if ((*ptp & ARM_PTP_VALID) == 0) {
			paddr_t				pa;
			struct pa_quantum	*qp;
			unsigned			status;
			memsize_t			resv = 0;
			part_id_t 		mpid;
			PROCESS *			prp = adp ? object_from_data(adp, address_cookie) : NULL;

			/*
			 * Only allocate a page table if we're installing a mapping
			 */
			if ((data->op & (PTE_OP_MAP|PTE_OP_PREALLOC|PTE_OP_BAD)) == 0) {
				va = (va + PDE_SPAN) & ~(PDE_SPAN-1);
				data->start = (data->start + PDE_SPAN) & ~(PDE_SPAN-1);
				continue;
			}

			mpid = mempart_getid(prp, sys_memclass_id);
			if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
				return ENOMEM;
			}
			qp = pa_alloc(__PAGESIZE, __PAGESIZE, PAQ_COLOUR_NONE, 0, &status, restrict_proc, resv);
			if (qp == NULL) {
				MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
				r = ENOMEM;
				break;
			}
			MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
			qp->flags |= PAQ_FLAG_SYSTEM;
			pa = pa_quantum_to_paddr(qp);
			if (status & PAA_STATUS_NOT_ZEROED) {
				ptzero(qp);
			}
			if (flags & PMF_USER_SPACE) {
				/*
				 * Add page table to address space l2_list
				 */
				qp->u.inuse.next = adp->cpu.l2_list;
				adp->cpu.l2_list = qp;

				ptmap_user(va, pa, flags & PMF_INACTIVE);
			}
			else {
				ptmap_kern(va, pa);
			}
		}

		/*
		 * If we're just preallocating page tables, move to next 4MB region
		 */
		if(data->op & PTE_OP_PREALLOC) {
			va = (va + PDE_SPAN) & ~(PDE_SPAN - 1);
			data->start = (data->start + PDE_SPAN) & ~(PDE_SPAN - 1);
			continue;
		}

		CRASHCHECK((*ptp & ARM_PTP_VALID) != ARM_PTP_V6_L2);

		if (flags & PMF_INACTIVE) {
			inactive_l2_map(*ptp);
			arm_v6_dsb();
		}

		/*
		 * Manipulate ptes in this 4MB region
		 */
		do {
			pte_t	opte;
			pte_t	npte;

			opte = npte = *pte;

			if (data->op & (PTE_OP_MAP|PTE_OP_BAD)) {
				npte = data->paddr | prot;
			}
			else if (data->op & PTE_OP_UNMAP) {
				npte = 0;
			}
			else if (opte) {	// PTE_OP_PROT
				/*
				 * Don't fiddle with the pte for PTE_OP_BAD mappings.
				 * These have (opte & PGMASK) set to ARM_XSP_PROT(ARM_PTE_RW).
				 *
				 * Regular mappings will be any of the following:
				 * - valid with RO permissions     (PROT_READ)
				 * - valid with RW permissions     (PROT_READ|PROT_WRITE|PROT_EXEC)
				 * - invalid with no permissions   (PROT_NONE)
				 */
				if ((opte & PGMASK) != ARM_PTE_BAD) {
					npte = (opte & ~PGMASK) | prot;
				}
			}
			if (npte != opte) {
				if (opte) {
					/*
					 * FIXME: if PMF_INACTIVE, and va is not mapped in the
					 *        current address space, the cache flush will
					 *        fault (at least on MPcore) because the TLB
					 *        lookup will fail.
					 *
					 * I think we're OK not flushing the cache because the
					 * mm_reference code takes care to purge the cache for
					 * colour mismatch or PROT_NOCACHE handling...
					 *
					 * Flush TLBs, passing addr + asid
					 */
					if(adp) {
						(void) arm_cpu->page_flush(va | adp->cpu.asid, 0);
					}
					else {
						(void) arm_cpu->page_flush(va, 0);
					}
#ifdef	VARIANT_smp
					flags |= PMF_FLUSH_SMP;
#endif
				}
				*pte = npte;
			}
			pte++;
			va += __PAGESIZE;
			data->start += __PAGESIZE;
			data->paddr += __PAGESIZE;

			if ((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
				r = EINTR;
				goto out;
			}
		} while (va < end && (va & (PDE_SPAN-1)) != 0);
	}

out:
	/*
	 * Complete any cache flush that couldn't be done by page_flush()
	 */
	if (flags & PMF_FLUSH_DEFER) {
		arm_cpu->page_flush_deferred(flags & PMF_FLUSH_CACHE);
	}
	
	if (flags & PMF_INACTIVE) {
		inactive_l1_unmap();
		inactive_l2_unmap();
		arm_v6_dsb();
	}
#ifdef	VARIANT_smp
	if (flags & PMF_FLUSH_SMP) {
		SMP_FLUSH_TLB();
	}
#endif
	return r;
}

__SRCVERSION("cpu_pte.c $Rev: 173587 $");
