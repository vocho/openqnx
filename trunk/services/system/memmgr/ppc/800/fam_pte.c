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
#include <ppc/800cpu.h>
#include <ppc/inline.h>


extern void		add_tlb800(uintptr_t, uintptr_t, uintptr_t, unsigned);

void
fam_pte_init(int phase) {
	// mmu 800 initialization. assuming int off, mmu off, called once only
	// MI_CTR & MD_CTR
	set_spr(PPC800_SPR_MI_CTR, 0/*PPC800_MICTR_CIDEF*/);
	set_spr(PPC800_SPR_MD_CTR, /*PPC800_MDCTR_WTDEF |*/	PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	
	// PID: M_CASID -- kernel
//	set_spr(PPC800_SPR_M_CASID, 1);
	set_spr(PPC800_SPR_M_CASID, 0);
	
	// ZONE: MI_AP & MD_AP
	set_spr(PPC800_SPR_MI_AP, 0x55555555);//PPC800_AP_PPCMODE_PGPERM(15) | PPC800_AP_PPCMODE_PGPERM(0));
	set_spr(PPC800_SPR_MD_AP, 0x55555555);//PPC800_AP_PPCMODE_PGPERM(0) |PPC800_AP_PPCMODE_PGPERM(15));

	fam_pte_flush_all();

	// set up cache control registers
	set_spr(PPC800_SPR_IC_CST, 0x0a000000);
	set_spr(PPC800_SPR_IC_CST, 0x0c000000);
	set_spr(PPC800_SPR_IC_CST, 0x02000000);
	ppc_isync();

	set_spr(PPC800_SPR_DC_CST, 0x0a000000);
	set_spr(PPC800_SPR_DC_CST, 0x0c000000);
	set_spr(PPC800_SPR_DC_CST, 0x02000000);
	ppc_isync();

#if 1
	// map in reserved tlb entries
	{
	unsigned		twc;
	unsigned		rpn;
	unsigned		epn;

	// add first 8M mapping for both ker and proc to both itlb and dtlb (31) 
	set_spr(PPC800_SPR_MI_CTR, (31<<PPC800_MICTR_INDX_SHIFT));
	set_spr(PPC800_SPR_MD_CTR, (31<<PPC800_MDCTR_INDX_SHIFT) | PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	epn = 0 | PPC800_EPN_EV;
	twc = PPC800_TWC_V | PPC800_TWC_PS_8M;
	rpn = 0 | PPC800_RPN_SH | PPC800_RPN_V | PPC800_RPN_LPS | 0xf0 | (0x1<<PPC800_RPN_PP2_SHIFT);
	add_tlb800( epn, twc, rpn, 1);
	ppc_isync();

	// add third 8M mapping for itlb (28), and second 8M for dtlb (28)
	set_spr(PPC800_SPR_MI_CTR, 28<<PPC800_MICTR_INDX_SHIFT);
	set_spr(PPC800_SPR_MD_CTR, (28<<PPC800_MDCTR_INDX_SHIFT) | PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	epn = 0x1000000  | PPC800_EPN_EV;
	rpn =  0x1000000  | PPC800_RPN_SH | PPC800_RPN_V | PPC800_RPN_LPS | 0xf0 | (0x1<<PPC800_RPN_PP2_SHIFT);
	add_tlb800( epn, twc, rpn, 1);
	
	set_spr(PPC800_SPR_MD_CTR, (28<<PPC800_MDCTR_INDX_SHIFT) | PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	epn = 0x800000  | PPC800_EPN_EV;
	rpn =  0x800000  | PPC800_RPN_SH | PPC800_RPN_V | PPC800_RPN_LPS | 0xf0 | (0x1<<PPC800_RPN_PP2_SHIFT);
	add_tlb800( epn, twc, rpn, 0);
	ppc_isync();


// It seems that tlb search takes much time for valid entries. So, keep tlb buffer simple.
	// add syspage for dtlb and itlb (29)
	set_spr(PPC800_SPR_MI_CTR, 29<<PPC800_MICTR_INDX_SHIFT);
	set_spr(PPC800_SPR_MD_CTR, (29<<PPC800_MDCTR_INDX_SHIFT) | PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	epn =  VM_SYSPAGE_ADDR | PPC800_EPN_EV;
	twc = PPC800_TWC_V;
	rpn = (unsigned)_syspage_ptr | PPC800_RPN_SH | PPC800_RPN_V | 0xf0 | (0x3<<PPC800_RPN_PP1_SHIFT)  |  (0x1<<PPC800_RPN_PP2_SHIFT);
	add_tlb800( epn, twc, rpn, 1);

	// add uncached 8M for dtlb, and second 8M for itlb (30)
	set_spr(PPC800_SPR_MI_CTR, 30<<PPC800_MICTR_INDX_SHIFT);
	set_spr(PPC800_SPR_MD_CTR, (30<<PPC800_MDCTR_INDX_SHIFT) | PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	epn = 0x800000 | PPC800_EPN_EV;
	twc = PPC800_TWC_V | PPC800_TWC_PS_8M;
	rpn = 0x800000 | PPC800_RPN_SH | PPC800_RPN_V | PPC800_RPN_LPS | 0xf0 | (0x1<<PPC800_RPN_PP2_SHIFT);
	add_tlb800( epn, twc, rpn, 1);

	set_spr(PPC800_SPR_MD_CTR, (30<<PPC800_MDCTR_INDX_SHIFT) | PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	epn =  0x30000000 | PPC800_EPN_EV;
	twc = PPC800_TWC_V | PPC800_TWC_PS_8M;
	rpn =  0x30000000 | PPC800_RPN_SH | PPC800_RPN_V | PPC800_RPN_LPS | 0xf0 | (0x1<<PPC800_RPN_PP2_SHIFT) | PPC800_RPN_CI;
	add_tlb800( epn, twc, rpn, 0);
	ppc_isync();

	// set reserved area boundry
	set_spr(PPC800_SPR_MI_CTR, PPC800_MICTR_PPCS | PPC800_MICTR_RSV2I );
	set_spr(PPC800_SPR_MD_CTR, PPC800_MDCTR_PPCS | PPC800_MDCTR_RSV2D | PPC800_MDCTR_TWAN | PPC800_MDCTR_CIDEF);
	}
#endif
	zp_flags &= ~ZP_CACHE_OFF;
	pgszlist[2] = __PAGESIZE;

}


void
fam_pte_flush_all() {
	ppc_tlbia();
	ppc_isync();
}


// This routine is only called by cpu_vmm_fault.
// assuming map a page (of system page length -4k)
void
fam_pte_mapping_add(uintptr_t vaddr, paddr_t paddr, unsigned prot, unsigned flags) {
	unsigned		twc;
	unsigned		rpn;
	unsigned		epn;

	epn = (vaddr & PPC800_EPN_EPN_MASK) | PPC800_EPN_EV; 
	twc = PPC800_TWC_V;
	rpn = (paddr & PPC800_RPN_RPN_MASK) | PPC800_RPN_V | 0xf0 /*| PPC800_RPN_CI*/;// turn off cache for the bug
	if(prot & PROT_WRITE) {
		rpn |= (0x2<<PPC800_RPN_PP1_SHIFT) | (0x1<<PPC800_RPN_PP2_SHIFT);
	} else {
		rpn |= 0x3<<PPC800_RPN_PP1_SHIFT;
	}
	if(prot & PROT_NOCACHE) {
		 rpn |= PPC800_RPN_CI;
		 twc |= PPC800_TWC_G;
	}

	//The 'flags' parm is the ASID in effect at the TLB miss.
   	//NYI: should respect VM_FAULT_GLOBAL flag (see book E implementation)
	add_tlb800((flags & 0xf) | epn, twc, rpn, flags & VM_FAULT_INSTR);
}


void
fam_pte_mapping_del(ADDRESS *adp, uintptr_t vaddr, unsigned size) {
	ppc_tlbia();
	ppc_isync();
}


/*
* fam_pte_asid_alloc:
*
* Assign an asid to a process- may steal one from
* another process. Since this code is called from
* virtual_map_address when switching in a process
* we are already in the kernel and there is no need
* for KerextAmInKernel checks.
*
*/

#define	PPC_ASID_BOUNDARY 15
static ADDRESS *asid_map[PPC_ASID_BOUNDARY+1];

// NIY: think about real int safe
void
fam_pte_asid_alloc(ADDRESS *adp) {
	static int	asid_rotor = 1;
	int 		asid;
	int			i;
	ADDRESS 	*oldadp;

	/*
	 * Be very, very careful in here. We can be allocating an asid for
	 * somebody and have an interrupt go off. If the interrupt has a
	 * handler routine, we will re-enterantly execute this code to set up
	 * the address space for the routine.
	 */
	
	/*
	 * Do a quick scan through the asid_map and see 
	 * if there are any unallocated entries. The
	 * reason why we do this is because it is cheaper 
	 * to do this one scan than to steal an asid,
	 * do a MemPageFlushAsid, and also possibly incur
	 * tlb refills for the poor guy we stole from.
	 * If there are no unallocated asids, then go 
	 * back to where we were in the asid_map.
	 */
	// asid 0 is reserved for system address space
    for (i = 1; i <= PPC_ASID_BOUNDARY; ++i) {

/*		if (asid_map[i] == 0
		  && (_smp_cmpxchg((unsigned *)(&asid_map[i]), 0, (unsigned)adp) == 0)) {
*/
		if (asid_map[i] == 0) {
			asid_map[i] = adp;
			adp->cpu.asid = i;
			return;
		}
    }
    /* have to steal one */
    asid = asid_rotor;
    if (++asid_rotor > PPC_ASID_BOUNDARY) {
		asid_rotor = 1;
    }
    adp->cpu.asid = asid;
//	oldadp = (ADDRESS *)_smp_xchg((unsigned *)&asid_map[asid], (unsigned)adp);
    oldadp = asid_map[asid];
	asid_map[asid] = adp;
    if (oldadp) {
		/* 
		 * Mark his asid as invalid so
		 * that when we switch to him he'll
		 * pick up another one.
		 */
		oldadp->cpu.asid = PPC_INVALID_ASID;
    }
	ppc_tlbia();
	ppc_isync();
}

void
fam_pte_asid_release(ADDRESS *adp) {
	int		flush = 0;

	// Have to do the following atomically, so that an interrupt can't
	// come in and invalidate the asid on us.
	InterruptDisable();
	if(adp->cpu.asid != PPC_INVALID_ASID) {
		asid_map[adp->cpu.asid] = NULL;
		adp->cpu.asid = PPC_INVALID_ASID;
		flush = 1;
	}
	InterruptEnable();
	if(flush) {
		ppc_tlbia();
		ppc_isync();
	}
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
	unsigned				l1bits;
	pte_t					**l1pagetable;
	struct pa_quantum		*pq;
	unsigned				pa_status;
	int						flushed;
	ADDRESS					*adp;
	int						r;
	part_id_t			mpid;
	PROCESS *				prp;

	// Assume alignment has been checked by the caller

	adp = data->adp;
	if(adp == NULL) crash();
	l1pagetable = adp->cpu.pgdir;

	l1bits = 0x1;  // validity bit
	if(data->op & PTE_OP_BAD) {
		bits = PPC800_RPN_CI;
	} else if(data->prot & (PROT_READ|PROT_WRITE|PROT_EXEC)) {
		bits = 0xf1;
		//RUSH3: if PTE_OP_TEMP, mark PTE as accessable from procnto only if possible
		if(data->prot & PROT_WRITE) {
			bits |= (0x2<<PPC800_RPN_PP1_SHIFT) | (0x1<<PPC800_RPN_PP2_SHIFT);
		} else if(data->prot & (PROT_READ|PROT_EXEC)) {
			bits |= 0x3<<PPC800_RPN_PP1_SHIFT;
		}
		if(data->shmem_flags & SHMCTL_HAS_SPECIAL) {
			if(data->special & ~PPC_SPECIAL_MASK) {
				return EINVAL;
			}
			//RUSH1: If PPC_SPECIAL_E/W/M/G is on, should I report an error?
			if((data->special & PPC_SPECIAL_I)) {
				bits |= PPC800_RPN_CI;
			}
		}
		if(data->prot & PROT_NOCACHE) {
			bits |= PPC800_RPN_CI;
			l1bits |= PPC800_TWC_G;
		}
	} else {
		bits = 0;
	}

	r = EOK;
	flushed = 0;
	prp = adp ? object_from_data(adp, address_cookie) : NULL;
	mpid = mempart_getid(prp, sys_memclass_id);
	for( ;; ) {
		if(data->start >= data->end) break;
		pdep = &l1pagetable[L1PAGEIDX(data->start)];
		l2_vaddr = (uintptr_t)*pdep;
		if(l2_vaddr == 0) {
			memsize_t  resv = 0;

			if(!(data->op & (PTE_OP_MAP|PTE_OP_PREALLOC|PTE_OP_BAD))) {
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
		}
		*pdep = (pte_t *)(l2_vaddr | l1bits);
		if(data->op & PTE_OP_PREALLOC) {
			//Move vaddr to next page directory
			data->start = (data->start + PDE_SIZE) & ~(PDE_SIZE - 1);
			if(data->start == 0) data->start = ~0;
			continue;
		}
		pdep_l2base = PDE_ADDR(*pdep);
		ptep = &(pdep_l2base[L2PAGEIDX(data->start)]);
		orig_pte = *ptep;
		if(data->op & (PTE_OP_MAP|PTE_OP_BAD)) {
			pte = data->paddr | bits;
		} else if(data->op & PTE_OP_UNMAP) {
			pte = 0;
		} else if(orig_pte & (0xfff & ~PPC800_RPN_CI)) {
			// PTE_OP_PROT
			pte = (orig_pte & ~0xfff) | bits;
		}  else {
			// We don't change PTE permissions if we haven't mapped the
			// page yet...
			pte = orig_pte;
		}
		*ptep = pte;
		if((orig_pte != 0) && (pte != orig_pte)) {
			flushed = 1;
			ppc_tlbie(data->start);
		}

		data->start += __PAGESIZE;
		data->paddr += __PAGESIZE;
		if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			r = EINTR;
			break;
		}
	}
	if(flushed) {
		ppc_isync();
	}
	return r;
}

__SRCVERSION("fam_pte.c $Rev: 172513 $");
