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

ptp_t					*L1_table;
struct arm_cpu_entry	*arm_cpu;

#define	MAX_ASID	256
static ADDRESS	*asid_map[MAX_ASID];

void
vx_capability_check(void) 
{
	/*
	 * Check compatibility with startup MMU configuration
	 */
	if ((SYSPAGE_ENTRY(cpuinfo)->flags & ARM_CPU_FLAG_V6) == 0) {
		kprintf("startup did not set ARM_CPU_FLAG_V6\n");
		crash();
	}
	if ((SYSPAGE_ENTRY(cpuinfo)->flags & ARM_CPU_FLAG_V6_ASID) == 0) {
		kprintf("startup did not set ARM_CPU_FLAG_V6_ASID\n");
		crash();
	}
}

int
cpu_vmm_fault(struct fault_info *info)
{
	PROCESS		*prp = info->prp;
	unsigned	vaddr = info->vaddr;
	unsigned	mva;
	pte_t		pte;

	/*
	 * Always return a fault for misaligned accesses:
	 * - without -ae the kernel will deliver the SIGBUS to the process
	 * - with -ae, the alignment emulation will retry the access using
	 *   safe accesses, so any translation/permission faults will be
	 *   caught then.
	 */
	if ((info->sigcode & 0xffff) == MAKE_SIGCODE(SIGBUS, BUS_ADRALN, 0)) {
		return -1;
	}

	mva = vaddr & ~PGMASK;

	/*
	 * Check if fault is accessing another address space via msgpass
	 */
	if ((info->sigcode & SIGCODE_INXFER) &&
		mva >= ARM_V6_XFER_BASE &&
		mva < ARM_V6_XFER_BASE + ARM_V6_XFER_SIZE) {
		struct xfer_slot	*slot = &xfer_slots[RUNCPU];

		if (slot->prp && (slot->size0 || slot->size1)) {
			info->prp = (PROCESS *)slot->prp;
			if (mva < ARM_V6_XFER_BASE + slot->size0) {
				info->vaddr = vaddr - slot->diff0;
			}
			else {
				info->vaddr = vaddr - slot->diff1;
			}
			return 0;
		}
	}

	/*
	 * Check if fault cannot be fixed up by fault_pulse()
	 * - read fault on a valid translation
	 * - write fault on a valid translation with write access
	 */
	pte = V6_USER_SPACE(mva) ? *UTOPDIR(mva) : *KTOPDIR(mva);
	if (pte & ARM_PTE_VALID) {
		pte = V6_USER_SPACE(mva) ? *UTOPTEP(mva) : *KTOPTEP(mva);
		if (pte & ARM_PTE_VALID) {
			/*
			 * WARNING: this code relies on the fact that the XSP protection
			 *          bits are the same as the first subpage protection
			 *          bits in an ARMv4 pte
			 */
			if (!(info->sigcode & SIGCODE_STORE) || ARM_PTE_WRITABLE(pte)) {
				return -1;
			}
		}
		else if ((pte & PGMASK) == ARM_PTE_BAD) {
			/*
			 * pte set for PTE_OP_BAD by cpu_pte_manipulate()
			 */
			return -2;
		}
	}

	/*
	 * Defer to process time to fixup fault.
	 * Update info so fault_pulse() knows what to try and fix up.
	 */

	//RUSH3: Does this have to be filled in? Who's using it?
	info->cpu.asid = prp ? prp->memory->cpu.asid : 0;
	info->prp = prp;
	info->vaddr = vaddr;
	return 0;
}

int
cpu_vmm_mcreate(PROCESS *prp)
{
	ADDRESS	*adp = prp->memory;
	struct pa_quantum	*pq;
	int					asid;
	unsigned			status;
	paddr_t				pa;
	memsize_t			resv = 0;
	part_id_t		mpid = mempart_getid(prp, sys_memclass_id);

	/*
	 * WARNING: we are currently called with kernel locked so no other
	 *          locks are required to manipulate asid_map[]
	 */
	for (asid = 0; asid < MAX_ASID; asid++) {
		if (asid_map[asid] == 0) {
			break;
		}
	}
	if (asid == MAX_ASID) {
		return EAGAIN;
	}
	asid_map[asid] = adp;
	adp->cpu.asid = asid;

	/*
	 * Allocate 8K L1 table to map 00000000-7fffffff
	 *
	 * The pte entries are marked non-global so we can map the L1 table
	 * at ARM_V6_USER_L1 without requiring a TLB flush on context switches
	 */
	if (MEMPART_CHK_and_INCR(mpid, 2*__PAGESIZE, &resv) != EOK) {
		return ENOMEM;
	}
	pq = pa_alloc(2*__PAGESIZE, 2*__PAGESIZE, PAQ_COLOUR_NONE, PAA_FLAG_CONTIG, &status, restrict_proc, resv);
	if (pq == NULL) {
		MEMPART_UNDO_INCR(mpid, 2*__PAGESIZE, resv);
		goto fail1;
	}
	MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), 2*__PAGESIZE);
	pa = pa_quantum_to_paddr(pq);
	adp->cpu.l1_pq = pq;
	adp->cpu.l1_pte = pa | l2_prot | ARM_PTE_V6_nG;
	if (status & PAA_STATUS_NOT_ZEROED) {
		ptzero(pq);
		ptzero(pq + 1);
	}
	/*
	 * FIXME_v6: some of the TTBR bits might be cpu specific?
	 */
#ifdef	VARIANT_smp
	adp->cpu.ttbr0 = pa | ARM_MMU_TTBR_S;
#else
	adp->cpu.ttbr0 = pa;
#endif

	/*
	 * Allocate 4K page directory
	 * Note that we only really use 2K (to map 00000000-7fffffff)
	 *
	 * The pte entries are marked non-global so we can map the L1 table
	 * at ARM_UPTE_BASE without requiring a TLB flush on context switches
	 */
	if (MEMPART_CHK_and_INCR(mpid, __PAGESIZE, &resv) != EOK) {
		return ENOMEM;
	}
	pq = pa_alloc(__PAGESIZE, __PAGESIZE, PAQ_COLOUR_NONE, PAA_FLAG_CONTIG, &status, restrict_proc, resv);
	if (pq == NULL) {
		MEMPART_UNDO_INCR(mpid, __PAGESIZE, resv);
		goto fail2;
	}
	MEMCLASS_PID_USE(prp, mempart_get_classid(mpid), __PAGESIZE);
	pa = pa_quantum_to_paddr(pq);
	adp->cpu.l2_pq = pq;
	adp->cpu.l2_pte = pa | l2_prot | ARM_PTE_V6_nG;
	adp->cpu.l2_ptp = pa | ARM_PTP_V6_L2;
	if (status & PAA_STATUS_NOT_ZEROED) {
		ptzero(pq);
	}

#ifdef	VARIANT_smp
	/*
	 * Indicate that vmm_aspace() needs to flush TLBs for this ASID
	 */
	adp->cpu.asid_flush = LEGAL_CPU_BITMASK;
#else
	/*
	 * Invalidate all (unified) TLB entries for our ASID
	 *
	 * FIXME_v6: current ARM11 and MPcore have unified TLBs
	 *        Need to check for other processors whether the unified op
	 *        will correctly invalidate both I and D TLBs...
	 */
	arm_v6_tlb_asid(adp->cpu.asid);
#endif

	adp->cpu.l2_list = 0;

	/*
	 * FIXME: need to figure out the correct thing here...
	 */
	adp->cpu.pgdir = L1_table;
	return EOK;

fail2:
	pa_free(adp->cpu.l1_pq, 2, MEMPART_DECR(mpid, 2*__PAGESIZE));
	MEMCLASS_PID_FREE(prp, mempart_get_classid(mpid), 2*__PAGESIZE);
fail1:
	asid_map[adp->cpu.asid] = 0;
	return EAGAIN;
}

void
cpu_vmm_mdestroy(PROCESS *prp)
{
	ADDRESS 	*adp = prp->memory;
	struct pa_quantum	*pq;
	struct pa_quantum	*nq;
	part_id_t		mpid = mempart_getid(prp, sys_memclass_id);
	memsize_t			memclass_pid_free = 0;

	/*
	 * Free page tables
	 */
	for (pq = adp->cpu.l2_list; pq; pq = nq) {
		nq = pq->u.inuse.next;
		pa_free(pq, 1, MEMPART_DECR(mpid, NQUANTUM_TO_LEN(1)));
		memclass_pid_free += NQUANTUM_TO_LEN(1);
	}

	/*
	 * Free L1 table and "page directory"
	 */
	if (adp->cpu.l1_pq) {
		pa_free(adp->cpu.l1_pq, 2, MEMPART_DECR(mpid, NQUANTUM_TO_LEN(2))); // 8K L1 table
		memclass_pid_free += NQUANTUM_TO_LEN(2);
	}
	if (adp->cpu.l2_pq) {
		pa_free(adp->cpu.l2_pq, 1, MEMPART_DECR(mpid, NQUANTUM_TO_LEN(1)));	// 4K L2 table
		memclass_pid_free += NQUANTUM_TO_LEN(1);
	}
	MEMCLASS_PID_FREE(prp, mempart_get_classid(mpid), memclass_pid_free);

	/*
	 * Release our ASID.
	 *
	 * WARNING: we are currently called with kernel locked so no other
	 *          locks are required to manipulate asid_map[]
	 */
	if (adp->cpu.asid) {
		asid_map[adp->cpu.asid] = 0;
	}
}


unsigned
cpu_vmm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *p, size_t *lenp)
{
	unsigned	pte;
	unsigned	pg_size;
	unsigned	pg_offset;
	unsigned	prot;

	prot = MAP_PHYS; // so PROT_NONE doesn't come back

	/*
	 * Check for section mapping
	 */
	pte = V6_USER_SPACE(vaddr) ? *UTOL1SC(vaddr) : *KTOL1SC(vaddr);
	if ((pte & 0x3) == (ARM_PTP_SC & 0x3)) {
		pg_offset = vaddr & ARM_SCMASK;
		*p = (pte & ~ARM_SCMASK) | pg_offset;

		if(lenp != NULL) {
			*lenp = ARM_SCSIZE - pg_offset;
		}

		prot |= PROT_READ | PROT_EXEC;

		if ((pte & ARM_PTP_V6_APX) == 0) {
			prot |= PROT_WRITE;
		}
		if ((pte & ARM_PTP_V6_XN) != 0) {
			prot &= ~PROT_EXEC;
		}
		if ((pte & ARM_PTP_C) == 0) {
			prot |= PROT_NOCACHE;
		}
	}
	else {
		pte = 0;
		if (V6_USER_SPACE(vaddr)) {
			if (*UTOPDIR(vaddr) & ARM_PTE_VALID) {
				pte = *UTOPTEP(vaddr);
			}
		}
		else {
			if (*KTOPDIR(vaddr) & ARM_PTE_VALID) {
				pte = *KTOPTEP(vaddr);
			}
		}
		if (!(pte & ARM_PTE_VALID)) {
			return PROT_NONE;
		}
		if ((pte & ARM_PTE_VALID) == ARM_PTE_LP) {
			pg_size   = ARM_LPSIZE;
			pg_offset = vaddr & ARM_LPMASK;
		}
		else {
			pg_size   = __PAGESIZE;
			pg_offset = ADDR_OFFSET(vaddr);
		}
		*p = (pte & ~(pg_size-1)) | pg_offset;

		if(lenp != NULL) {
			*lenp = pg_size - pg_offset;
		}

		// all valid translations are readable and executable
		prot |= PROT_READ | PROT_EXEC;

		if ((pte & ARM_PTE_V6_APX) == 0) {
			prot |= PROT_WRITE;
		}
		if ((pte & ARM_PTE_VALID) == ARM_PTE_LP) {
			if ((pte & ARM_PTE_V6_LP_XN) != 0) {
				prot &= ~PROT_EXEC;
			}
		}
		else {
			if ((pte & ARM_PTE_V6_SP_XN) != 0) {
				prot &= ~PROT_EXEC;
			}
		}
		if ((pte & ARM_PTE_C) == 0) {
			prot |= PROT_NOCACHE;
		}
	}
	return prot;
}

__SRCVERSION("cpu_vmm.c $Rev: 173587 $");
