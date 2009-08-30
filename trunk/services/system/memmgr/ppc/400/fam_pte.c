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
#include <ppc/400cpu.h>


#define PPC_ASID_FIRST	2		// 0 is global, 1 is for kernel
#define NUM_ASIDS		256
static ADDRESS	*asid_map[NUM_ASIDS];

void
fam_pte_init(int phase) {
	set_spr(PPC400_SPR_ZPR, PPC400_ZPR_Zx_PNSG(1) | PPC400_ZPR_Zx_PGSG(2));
	ppc_tlbia();
	ppc_isync();
	zp_flags &= ~ZP_CACHE_OFF;
	pgszlist[0] = __PAGESIZE;
}


void
fam_pte_flush_all() {
	ppc_tlbia();
	ppc_isync();
}


void
fam_pte_mapping_add(uintptr_t vaddr, paddr_t paddr, unsigned prot, unsigned flags) {
	extern void		add_tlb400(uintptr_t, uintptr_t, unsigned);
	unsigned		lo;
	unsigned		hi;
	unsigned		zone;

	//The 'flags' parm is the PID in effect at the TLB miss.
	if(flags & VM_FAULT_GLOBAL) {
		flags = 0;
	} else {
		flags &= 0xff;
	}
	hi = vaddr + PPC400_TLBHI_SIZE_4K + PPC400_TLBHI_VALID;
	zone = (flags == 1) ? 1 : 2;
	lo = paddr + (zone << PPC400_TLBLO_ZONE_SHIFT);
	if(prot & PROT_WRITE) lo |= PPC400_TLBLO_WR;
	if(prot & PROT_EXEC)  lo |= PPC400_TLBLO_EX;
	if(prot & PROT_NOCACHE)  {
		lo |= PPC400_TLBLO_I;
		if(!(prot & PROT_EXEC)) lo |= PPC400_TLBLO_G;
	}
	add_tlb400(hi, lo, flags);
}

void
fam_pte_mapping_del(ADDRESS *adp, uintptr_t vaddr, unsigned size) {
	//NYI: if size is small enough, should just flush the vaddr.
	//See Book E implementation.
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

// NIY: think about real int safe
void
fam_pte_asid_alloc(ADDRESS *adp) {
	static int	asid_rotor = PPC_ASID_FIRST;
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
    for(i = PPC_ASID_FIRST; i < NUM_ASIDS; ++i) {

/*		if (asid_map[i] == 0
		  && (_smp_cmpxchg((unsigned *)(&asid_map[i]), 0, (unsigned)adp) == 0)) {
*/
//kprintf("i=%x, asid_map[i]=%x\n",i,asid_map[i]);
		if(asid_map[i] == 0) {
			asid_map[i] = adp;
			adp->cpu.asid = i;
			return;
		}
    }
    /* have to steal one */
    if(++asid_rotor >= NUM_ASIDS) {
		asid_rotor = PPC_ASID_FIRST;
    }
    asid = asid_rotor;
    adp->cpu.asid = asid;
//	oldadp = (ADDRESS *)_smp_xchg((unsigned *)&asid_map[asid], (unsigned)adp);
    oldadp = asid_map[asid];
	asid_map[asid] = adp;
    if(oldadp) {
		/* 
		 * Mark his asid as invalid so
		 * that when we switch to him he'll
		 * pick up another one.
		 */
		oldadp->cpu.asid = PPC_INVALID_ASID;
    }
	//tlbop.flush_asid(asid);
	ppc_tlbia(); ppc_isync();
}

void
fam_pte_asid_release(ADDRESS *adp) {
	int		flush;

	// Have to do the following atomically, so that an interrupt can't
	// come in and invalidate the asid on us.
	InterruptDisable();
	flush = adp->cpu.asid;
	if(flush != PPC_INVALID_ASID) {
		adp->cpu.asid = PPC_INVALID_ASID;
		asid_map[flush] = NULL;
	}
	InterruptEnable();
	if(flush != PPC_INVALID_ASID) {
		//tlbop.flush_asid(flush);
		ppc_tlbia(); ppc_isync();
	}
}


int
cpu_pte_manipulate(struct mm_pte_manipulate *data) {
	pte_t					**pdep;
	pte_t					*pdep_l2base;
	pte_t					*ptep;
	pte_t					orig_pte;
	pte_t					pte;
	uintptr_t				l2_vaddr;
	unsigned				bits;
	pte_t					**l1pagetable;
	int						smp_flush = 0;
	struct pa_quantum		*pq;
	unsigned				pa_status;
	unsigned				zone;
	ADDRESS					*adp;
	int						r;
	part_id_t			mpid;
	PROCESS *				prp;

	// Assume alignment has been checked by the caller

	adp = data->adp;
	if(adp == NULL) crash();
	l1pagetable = adp->cpu.pgdir;

	zone = 0;
	if(data->op & PTE_OP_BAD) {
		bits = PPC400_TLBLO_I;
	} else {
		bits = 0;
		//RUSH3: if PTE_OP_TEMP, mark PTE as accessable from procnto only if possible
		if(data->prot & (PROT_EXEC|PROT_READ|PROT_WRITE)) {
			if(data->shmem_flags & SHMCTL_HAS_SPECIAL) {
				if(data->special & ~PPC_SPECIAL_MASK) {
					return EINVAL;
				}
				// Downshift by one because PPC_SPECIAL_G is in bit position 30, but
				// The PPC400_TLBLO_G bit is position 31.
				bits |= data->special >> 1;
				//RUSH1: Have to hide PPC_SPECIAL_E somewhere in TLBLO (it's actually
				//RUSH1: stored in TLBHI :-(). Top bit of zone field looks promising.
			}
			if(data->prot & PROT_EXEC) {
				bits |= PPC400_TLBLO_EX;
			}
			if(data->prot & PROT_WRITE) {
				bits |= PPC400_TLBLO_WR;
			}
			if(data->prot & PROT_NOCACHE) {
				bits |= PPC400_TLBLO_I;
				if(!(data->prot & PROT_EXEC)) {
					bits |= PPC400_TLBLO_G;
				}
			}
			if(WITHIN_BOUNDRY(data->start, data->start, user_boundry_addr)) {
				zone = 2;
			} else {
				zone = 1;
			}
		}
	}
	bits |= zone << PPC400_TLBLO_ZONE_SHIFT;

	r = EOK;
	prp = adp ? object_from_data(adp, address_cookie) : NULL;
	mpid = mempart_getid(prp, sys_memclass_id);
	for( ;; ) {
		if(data->start >= data->end) break;
		pdep = &l1pagetable[L1PAGEIDX(data->start)];
		if(*pdep == NULL) {
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
			*pdep = (pte_t *)l2_vaddr;
		}
		if(data->op & PTE_OP_PREALLOC) {
			//Move vaddr to next page directory
			data->start = (data->start + PDE_SIZE) & ~(PDE_SIZE - 1);
			if(data->start == 0) data->start = ~0;
			continue;
		}
		pdep_l2base = *pdep;
		ptep = &(pdep_l2base[L2PAGEIDX(data->start)]);
		orig_pte = *ptep;
		if(data->op & (PTE_OP_MAP|PTE_OP_BAD)) {
			pte = data->paddr | bits;
		} else if(data->op & PTE_OP_UNMAP) {
			pte = 0;
		} else if(orig_pte & (0xfff & ~PPC400_TLBLO_I)) {
			// PTE_OP_PROT
			pte = (orig_pte & ~0xfff) | bits;
		}  else {
			// We don't change PTE permissions if we haven't mapped the
			// page yet...
			pte = orig_pte;
		}
		*ptep = pte;
		if((orig_pte != 0) && (pte != orig_pte)) {
			//NYI: should just flush the vaddr...
			smp_flush = 1;
		}

		data->start += __PAGESIZE;
		data->paddr += __PAGESIZE;
		if((data->op & PTE_OP_PREEMPT) && KerextNeedPreempt()) {
			r = EINTR;
			break;
		}
	}
	if(smp_flush) {
		ppc_tlbia();
		ppc_isync();
	}
	return r;
}

__SRCVERSION("fam_pte.c $Rev: 172513 $");
