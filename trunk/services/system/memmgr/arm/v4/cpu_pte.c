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

/*
 * Convert a large page L2 descriptor to small page L2 descriptor
 */
static unsigned
lp_to_sp(pte_t opte)
{
	unsigned	npte;

	npte = opte & ~ARM_PTE_VALID;

	/*
	 * Figure out if we need to set up ARMv4 small page or ARMv5
	 * extended small page entries.
	 */
	if ((arm_cpu->upte_ro & ARM_PTE_VALID) == ARM_PTE_XSP ||
		(arm_cpu->upte_rw & ARM_PTE_VALID) == ARM_PTE_XSP ||
		(arm_cpu->kpte_ro & ARM_PTE_VALID) == ARM_PTE_XSP ||
		(arm_cpu->kpte_rw & ARM_PTE_VALID) == ARM_PTE_XSP) {
		/*
		 * Xscale extended small page: use TEX + AP field only
		 */
		npte &= ~0xffc0;				// clear TEX and AP3/AP2/AP1 fields
		npte |= (opte & ARM_PTE_V5_LP_TEX_MASK) >> 6;	// set TEX bits
		npte |= ARM_PTE_XSP;
	}
	else {
		/*
		 * Descriptors only differ in paddr and valid bits
		 */
		npte |= ARM_PTE_SP;
	}
	return npte;
}

/*
 * Convert a small page L2 descriptor to large page L2 descriptor
 */
static unsigned
sp_to_lp(pte_t opte)
{
	unsigned	npte;

	if ((opte & ARM_PTE_VALID) == ARM_PTE_XSP) {
		unsigned prot;

		npte = opte & ~0xfff0;		// clear paddr 15:12, TEX and AP bits
		prot = (opte >> 4) & 3;		// AP bits
		npte |= ARM_PTE_PROT(prot);
		prot |= (opte & ARM_PTE_V5_SP_TEX_MASK) << 6;	// TEX bits
	}
	else {
		npte = opte & ~0xf000;		// clear paddr bits 15:12
	}
	npte = (npte & ~ARM_PTE_VALID) | ARM_PTE_LP;

	return npte;
}

/*
 * Convert L2 descriptor into L1 section descriptor
 */
static unsigned
l2_to_sc(pte_t opte)
{
	unsigned	ptp;

	ptp  = opte & ~ARM_SCMASK;		// paddr
	ptp |= opte & ARM_PTE_CB;
	ptp |= ARM_PTP_SC;
	if ((opte & ARM_PTE_VALID) == ARM_PTE_XSP) {
		ptp |= (opte & 0x3f0) << 6;	// TEX + AP bits
	}
	else {
		ptp |= (opte & 0x030) << 6;	// AP bits
	}
	return ptp;
}

static int
ptalloc(PROCESS *prp, unsigned va, unsigned domain)
{
	paddr_t				pa;
	pte_t				*pte = VTOPDIR(va);
	ptp_t				*ptp = VTOL1PT(va);
	struct pa_quantum	*qp;
	unsigned			status;
	memsize_t			resv = 0;
	part_id_t		mpid = mempart_getid(prp, sys_memclass_id);

	if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
		return ENOMEM;
	}
	qp = pa_alloc(__PAGESIZE, __PAGESIZE, PAQ_COLOUR_NONE, 0, &status, restrict_proc, resv);
	if (qp == NULL) {
		MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
		return ENOMEM;
	}
	MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
	qp->flags |= PAQ_FLAG_SYSTEM;
	pa = pa_quantum_to_paddr(qp);

	/*
	 * Map page table into the "page directory"
	 */
	*pte = (pa | arm_cpu->kpte_rw) & ~arm_cpu->mask_nc;

	/*
	 * Create L1 entries
	 */
	pa |= ARM_PTP_L2;
	PTP_DOMAIN_SET(pa, domain);
	*ptp++ = pa; pa += ARM_L2_SIZE;
	*ptp++ = pa; pa += ARM_L2_SIZE;
	*ptp++ = pa; pa += ARM_L2_SIZE;
	*ptp = pa;

	if (status & PAA_STATUS_NOT_ZEROED) {
		CPU_ZERO_PAGE(VTOPTP(va), __PAGESIZE, 0);
	}
	return 0;
}

int
cpu_pte_split(uintptr_t vaddr, struct mm_pte_manipulate *data)
{
	unsigned	mva;
	unsigned	end;
	unsigned	mva_base;

	if (data->op & PTE_OP_MERGESTARTED) {
		/*
		 * We took care of everything with the first call
		 */
		return EOK;
	}

	/*
	 * Work on 1MB boundaries to break up section mappings if necessary
	 */
	mva_base = (vaddr < USER_SIZE) ? MVA_BASE(data->adp->cpu.asid) : 0;
	mva = ROUNDDOWN(vaddr, ARM_SCSIZE) + mva_base;
	end = data->end + mva_base;
	while (mva < end) {
		pte_t		*pte = VTOPDIR(mva);

		if ((*pte & ARM_PTE_VALID) == 0) {
			/*
			 * No page tables covering this area
			 */
			mva += ARM_SCSIZE;
		}
		else {
			/*
			 * Check if we need to split a section mapping
			 */
			ptp_t		*ptp = VTOL1SC(mva);

			if ((*ptp & ARM_PTP_VALID) == (ARM_PTP_SC & ARM_PTP_VALID)) {
				unsigned	l2_addr;

				/*
				 * Set L1 descriptor back to a L2 page table pointer
				 */
				l2_addr  = (*pte & ~PGMASK);
				l2_addr += ((mva >> 20) & 3) * ARM_L2_SIZE;
				l2_addr |= ARM_PTP_L2;
				PTP_DOMAIN_SET(l2_addr, PTP_DOMAIN(*ptp));

				*ptp = l2_addr;		
				arm_v4_idtlb_addr(mva);

				/*
				 * Update data->split_end so cpu_pte_merge() can attempt to
				 * merge mappings in this 1MB region beyond data->end
				 */
				data->split_end = (mva - mva_base) + ARM_SCSIZE - 1;
			}
			CRASHCHECK((*ptp & ARM_PTP_VALID) != (ARM_PTP_L2 & ARM_PTP_VALID));

			/*
			 * Check if we need to split large page mappings
			 */
			mva = ROUNDDOWN(vaddr, ARM_LPSIZE) + mva_base;
			pte = VTOPTEP(mva);
			do {
				if ((*pte & ARM_PTE_VALID) == ARM_PTE_LP) {
					unsigned	npte  = lp_to_sp(*pte);
					int			i;

					for (i = 0; i < 16; i++) {
						pte[i] = npte;
						npte += __PAGESIZE;
					}
					arm_v4_idtlb_addr(mva);
				}
				mva += ARM_LPSIZE;
				pte += 16; 
			} while (mva < end && (mva & ARM_SCMASK) != 0);
		}

		/*
		 * mva is now at next 1MB boundary or the 64K boundary following end
		 */
		vaddr = mva - mva_base;
		if (vaddr > data->split_end) {
			data->split_end = vaddr - 1;
		}

		if ((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			data->start = vaddr;
			return EINTR;
		}
	}

	/*
	 * Everything has been split
	 */
	data->op |= PTE_OP_MERGESTARTED;
	return EOK;
}

int
cpu_pte_merge(struct mm_pte_manipulate *data)
{
	unsigned	vaddr = data->start;
	unsigned	mva;
	unsigned	end;
	unsigned	mva_base;

	/*
	 * Work on 1MB boundaries to break up section mappings if necessary
	 */
	mva_base = (vaddr < USER_SIZE) ? MVA_BASE(data->adp->cpu.asid) : 0;
	mva = ROUNDDOWN(vaddr, ARM_SCSIZE) + mva_base;
	end = data->split_end + mva_base;
	while (mva < end) {
		pte_t		*pte;
		ptp_t		*ptp;

		/*
		 * Nothing to do if area is unmapped or already has section mapping
		 */
		pte = VTOPDIR(mva);
		ptp = VTOL1SC(mva);
		if ((*pte & ARM_PTE_VALID) == 0 || (*ptp & ARM_PTP_VALID) == (ARM_PTP_SC & ARM_PTP_VALID)) {
			/*
			 * Area is unmapped or already has a section mapping
			 */
			mva += ARM_SCSIZE;
		}
		else {
			pte_t		opte;
			pte_t		npte;
			unsigned	size;
			int			npg;

			/*
			 * Check if we can coalesce this 1MB region into a section entry:
			 * - physical address must be 1MB aligned
			 * - all ptes must map physically contiguous addresses
			 * - all ptes must have same access protections
			 *
			 * To keep the contiguity checks simple, we convert large page
			 * descriptors to small page descriptors. This means we can
			 * simply add the size of the mapping to each entry to determine
			 * the required value of the next descriptor.
			 */
			pte = VTOPTEP(mva);
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
				opte = l2_to_sc(*pte);
				PTP_DOMAIN_SET(opte, PTP_DOMAIN(*ptp));
				*ptp = opte;
				npg = 0;
				while (npg < ARM_SCSIZE / __PAGESIZE) {
					arm_v4_idtlb_addr(mva);
					if ((pte[npg] & ARM_PTE_VALID) == ARM_PTE_LP) {
						size = ARM_LPSIZE;
						npg += 16;
					}
					else {
						size = __PAGESIZE;
						npg++;
					}
					mva += size;
				}
			}
			else {
				/*
				 * Check if we can coalesce into large pages:
				 * - physical address must be 64K aligned
				 * - all ptes must map physically contiguous addresses
				 * - all ptes must have same access protections
				 */
				mva = ROUNDDOWN(vaddr, ARM_LPSIZE) + mva_base;
				pte = VTOPTEP(mva);
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
							unsigned	tva  = mva;

							npte = sp_to_lp(optelocal);
							for (npg = 0; npg < 16; npg++) {
								pte[npg] = npte;
								arm_v4_idtlb_addr(tva);
								tva += __PAGESIZE;
							}
						}
					}
					mva += ARM_LPSIZE;
					pte += 16;
				} while (mva < end && (mva & ARM_SCMASK) != 0);
			}
		}

		/*
		 * mva is now at next 1MB boundary or the 64K boundary following end
		 */
		vaddr = mva - mva_base;
		if ((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			data->start = vaddr;
			return EINTR;
		}
	}
	return EOK;
}

int
cpu_pte_manipulate(struct mm_pte_manipulate *data)
{
	ADDRESS		*adp = data->adp;
	unsigned	va  = data->start;
	unsigned	end = data->end;
	unsigned	prot = 0;
	unsigned	domain = 0;
	unsigned	cflush = 0;
	unsigned	dflush = 0;
	int			r;
	PROCESS *	prp = adp ? object_from_data(adp, address_cookie) : NULL;

	// Assume alignment has been checked by the caller

	/*
	 * Figure out page table access protection if necessary
	 */
	if (data->op & PTE_OP_BAD) {
		/*
		 * We will create an invalid pte with a known signature that
		 * is detected by cpu_vmm_fault()
		 */
		prot = ARM_PTE_BAD;
	}
	else if (data->op & (PTE_OP_MAP|PTE_OP_PROT)) {
		int		user_prot;

		/*
		 * FIXME_SUNIL: ARM's virtual cache means MAP_SHARED mappings must be
		 *              forced to PROT_NOCACHE to ensure we don't get aliases.
		 *              We don't need to do this for MAP_ELF (executable images)
		 *              or for SHMCTL_GLOBAL mappings (which have same virtual
		 *              address in all address spaces).
		 *
		 *              Need to check if doing the test here does not result in
		 *              over-zealously making this uncached.
		 */
		if ((data->prot & MAP_TYPE) == MAP_SHARED &&
		    (data->prot & MAP_ELF) == 0 &&
		    (data->shmem_flags & SHMCTL_GLOBAL) == 0) {
			data->prot |= PROT_NOCACHE;
		}

		/*
		 * FIXME: PTE_OP_TEMP mappings should also be considered privileged,
		 *        but the kernel message pass code uses user privileges if
		 *        the destination is a "user" address. This will cause any
		 *        proc_read() to these temp mappings to fault.
		 *
		 *        This will create a window where a multithreaded process
		 *        may find access to this temp mapping while the page is
		 *        being initialised, instead of faulting.
		 */
		if (va < USER_SIZE) {
			/*
			 * Normal user address space
			 */
			user_prot = 1;
		}
		else if (ARM_GBL_MAPPING(va)) {
			/*
			 * Global address space.
			 * Access is specified by shmem_flags:
			 * SHMCTL_PRIV      -> unprotected (privileged access)
			 * SHMCTL_LOWERPROT -> unprotected (user access)
			 * otherwise        -> protected   (user access)
			 */
			if (ARM_GBL_PRIVILEGED(data->shmem_flags)) {
				user_prot = 0;
			}
			else {
				user_prot = 1;
			}
			if (ARM_GBL_PROTECTED(data->shmem_flags) && adp != NULL) {
				domain = adp->cpu.domain;
			}
		}
		else {
			/*
			 * kernel address range
			 */
			user_prot = 0;
		}

		/*
		 * Figure out access protections to use
		 */
		if (user_prot) {
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
		if (data->shmem_flags & SHMCTL_LAZYWRITE) {
			/*
			 * Implement "lazy writing" as uncached/bufferable: TEX=0,C=0,B=1
			 *
			 * This provides uncached access but memory writes can be merged
			 * into write buffer to avoid the cpu stalling for normal uncached
			 * (TEX=0,C=0,B=1) access.
			 */
			prot &= ~arm_cpu->mask_nc;
			prot |= ARM_PTE_B;
		}
		if (data->shmem_flags & SHMCTL_HAS_SPECIAL) {
			/*
			 * Allow custom setting of S/TEX/C/B bits
			 *
			 * Note that S/TEX bits apply only for ARMv5 extended small pages.
			 */
			#define PTE_SPECIAL_BITS	(ARM_PTE_V5_SP_TEX_MASK | ARM_PTE_CB)

			prot &= ~PTE_SPECIAL_BITS;
			prot |= data->special & PTE_SPECIAL_BITS;
		}
	}

	/*
	 * Calculate MVA if necessary
	 */
	if (va < USER_SIZE) {
		va  |= MVA_BASE(adp->cpu.asid);
		end |= MVA_BASE(adp->cpu.asid);
		domain = adp->cpu.domain;
	}

	r = EOK;
	while (va < end) {
		ptp_t	*ptp;
		pte_t	*pte;

		ptp = VTOL1SC(va);
		pte = VTOPTEP(va);

		/*
		 * Check if we have page tables covering this 4MB region
		 */
		if ((*ptp & ARM_PTP_VALID) == 0) {

			/*
			 * Only allocate a page table if we're installing a mapping
			 */
			if ((data->op & (PTE_OP_MAP|PTE_OP_PREALLOC|PTE_OP_BAD)) == 0) {
				va = (va + PDE_SPAN) & ~(PDE_SPAN-1);
				data->start = (data->start + PDE_SPAN) & ~(PDE_SPAN-1);
				continue;
			}
			if (ptalloc(prp, va, domain) != 0) {
				return ENOMEM;
			}
		}
		else if ((data->op & (PTE_OP_MAP|PTE_OP_PROT|PTE_OP_BAD)) && ARM_GBL_MAPPING(va)) {
			unsigned	tva = va & ~ARM_SCMASK;
			ptp_t		*ptplocal = ptp;

			/*
			 * Set the L1 entry domain field to our domain if necessary
			 */
			do {
				ptp_t		ptmp = *ptplocal;

				if (PTP_DOMAIN(ptmp) != domain) {
					PTP_DOMAIN_CLR(ptmp);
					PTP_DOMAIN_SET(ptmp, domain);
					*ptplocal = ptmp;
				}
				ptplocal++;
				tva += ARM_SCSIZE;
			} while (tva < end && (tva & (PDE_SPAN-1)) != 0);
		}

		/*
		 * If we're just preallocating page tables, move to next 4MB region
		 */
		if(data->op & PTE_OP_PREALLOC) {
			va = (va + PDE_SPAN) & ~(PDE_SPAN - 1);
			data->start = (data->start + PDE_SPAN) & ~(PDE_SPAN - 1);
			continue;
		}

		/*
		 * cpu_pte_split should have ensured that everything is split up
		 * so we can do page level manipulation
		 */
		CRASHCHECK((*ptp & ARM_PTP_VALID) != (ARM_PTP_L2 & ARM_PTP_VALID));

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
					 * Flush cache and TLB if physical address of mapping changed.
					 * Otherwise just flush the TLB(s).
					 */
					if ((opte & ARM_PTE_C) && (npte & ~PGMASK) != (opte & ~PGMASK)) {
						cflush = 1;
						dflush |= arm_cpu->page_flush(va, 1);
					}
					else {
						dflush |= arm_cpu->page_flush(va, 0);
					}
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
	if (dflush) {
		arm_cpu->page_flush_deferred(cflush);
	}
	return r;
}

__SRCVERSION("cpu_pte.c $Rev: 210420 $");
