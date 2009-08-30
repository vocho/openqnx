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


int 
cpu_vmm_fault(struct fault_info *info) {
	PROCESS			*prp;
	ADDRESS			*adp;
	uintptr_t		vaddr;
	pte_t			*ptep;
	pte_t			*pde;
	pte_t			**root = 0;
	unsigned		flags;
	int 			index;
	uintptr_t 		real_addr;

	vaddr = info->vaddr;
	//RUSH3: use info->cpu.asid appropriately
	flags = info->cpu.code | info->cpu.asid;
	// First check for cpupage/syspage faults, as these don't require an aspace
	if(!(flags & VM_FAULT_WRITE)) {
		if(vaddr >= VM_SYSPAGE_ADDR && vaddr < (VM_SYSPAGE_ADDR+__PAGESIZE)) {
			fam_pte_mapping_add(VM_SYSPAGE_ADDR,
				(uintptr_t)_syspage_ptr,
				PROT_EXEC | PROT_READ, flags | VM_FAULT_GLOBAL);
			return 1;
		}
		if(!(flags & VM_FAULT_INSTR) && vaddr >= VM_CPUPAGE_ADDR && vaddr < (VM_CPUPAGE_ADDR+__PAGESIZE)) {
			struct system_private_entry	*pp;

			pp = SYSPAGE_ENTRY(system_private);
			fam_pte_mapping_add(VM_CPUPAGE_ADDR,
				(uintptr_t)pp->kern_cpupageptr + (RUNCPU * pp->cpupage_spacing),
				PROT_READ, flags | VM_FAULT_GLOBAL);
			return 1;
		}
	}

	prp = info->prp;
	if((adp = prp->memory) == NULL || (root = adp->cpu.pgdir) == NULL) {
		crash();
	}
#if defined(VARIANT_600)	
	if((flags & VM_FAULT_INSTR) && (flags & VM_FAULT_WIMG_ERR)) {
		unsigned	sr;

		sr = get_sreg(vaddr);
		if((sr & PPC_SR_N) && ((adp->cpu.nx_state & (1 << (vaddr >> 28))) == 0)) {
			// The segment register has the N bit on, but the nx_state
			// says it should be off - update and try the access again
			set_sreg(vaddr, sr & ~PPC_SR_N);
			return 1;
		}
	}
#endif				
	pde = PDE_ADDR(root[L1PAGEIDX(vaddr)]);
	if(pde != NULL) {
		ptep = &pde[L2PAGEIDX(vaddr)];
		if(!PTE_PRESENT(ptep) && !PTE_CACHEABLE(ptep)) {
			// cpu_pte_manipulate has marked the page as 'bad'
			return -2;
		}
	}

	if( vaddr >= VM_MSG_XFER_START
	 && vaddr <  VM_MSG_XFER_END
	 && (info->sigcode & SIGCODE_INXFER)) {
	    //Doing a message transfer
		prp = xfer_prp;
		if((adp = prp->memory) == NULL || (root = adp->cpu.pgdir) == NULL) {
			crash();
		}
		index = (vaddr - VM_MSG_XFER_START) >> 28;
		real_addr = vaddr - xfer_diff[index];
		info->prp = prp;
		info->vaddr = real_addr;

		//RUSH3: If we change memmgr/ppc/vmm_map_xfer.c to punch the
		//RUSH3: L1 pgtbl pointers into the active aspace, we wouldn't
		//RUSH3: need this code here and msg passing would be faster.
		pde = PDE_ADDR(root[L1PAGEIDX(real_addr)]);
		if(pde != NULL) {
			int prot;

			ptep = &pde[L2PAGEIDX(real_addr)];

			if(PTE_PRESENT(ptep)) {
				if(PTE_READABLE(ptep)) {
					prot = PROT_READ;
					if(PTE_WRITEABLE(ptep)) prot |= PROT_WRITE;
					if(!PTE_CACHEABLE(ptep)) prot |= PROT_NOCACHE;
					if((prot & PROT_WRITE) || (!(flags & VM_FAULT_WRITE))) {
						paddr_t		paddr;

						// not the protection violation for write
						xfer_rotor = (xfer_rotor + 1) & (NUM_XFER_MAPPINGS - 1);
						if(xfer_lastaddr[xfer_rotor]) {
							fam_pte_mapping_del(adp, xfer_lastaddr[xfer_rotor], 
									__PAGESIZE);
						}
						if(xfer_rotor > 1) crash();
						xfer_lastaddr[xfer_rotor] = ADDR_PAGE(vaddr);
						paddr = PTE_PADDR(ptep)
							+ (real_addr & ((PTE_PGSIZE(ptep) - 1) & ~(__PAGESIZE-1)));
						fam_pte_mapping_add(ADDR_PAGE(vaddr), paddr, prot, flags);
						return 1;
					}
				}
			} else if(!PTE_CACHEABLE(ptep)) {
				// Page was marked as 'bad'
				return -2;
			}
		}
		vaddr = real_addr;
	}

	if(vaddr < CPU_USER_VADDR_START) {
		// Address in Proc's address space
		return -1;
	}

	return 0;
}

#define L1TBL_SIZE	((1 << (32-L1_SHIFT))*sizeof(pte_t *))

int
cpu_vmm_mcreate(PROCESS *prp) {
	ADDRESS				*adp;
	void				*vaddr;
	size_t				size;
	part_id_t		mpid = mempart_getid(prp, sys_memclass_id);
	adp = prp->memory;
	if(vmm_mmap(NULL, 0, L1TBL_SIZE, PROT_READ | PROT_WRITE, 
				MAP_PRIVATE|MAP_ANON|MAP_PHYS, 0, 0, __PAGESIZE, 0, NOFD, &vaddr, &size, mpid) != EOK) {
		return ENOMEM;
	}
	adp->cpu.asid = PPC_INVALID_ASID;
	if(!PPC_INIT_ASID(adp)) {
		// fail
		(void)vmm_munmap(NULL, (uintptr_t)vaddr, L1TBL_SIZE, 0, mpid);
		return ENOMEM;
	}
	zero_page(vaddr, L1TBL_SIZE, NULL);
	adp->cpu.pgdir = vaddr;

	return EOK;
}


void
cpu_vmm_mdestroy(PROCESS *prp) {
	ADDRESS				*adp;
	struct pa_quantum	*curr;
	struct pa_quantum	*next;
	part_id_t mpid = mempart_getid(prp, sys_memclass_id);
	memsize_t			memclass_pid_free = 0;

	adp = prp->memory;

	curr = adp->cpu.l2_list;
	while(curr != NULL) {
		next = curr->u.inuse.next;
		pa_free(curr, 1, MEMPART_DECR(mpid, NQUANTUM_TO_LEN(1)));
		memclass_pid_free += NQUANTUM_TO_LEN(1);
		curr = next;
	}
	MEMCLASS_PID_FREE(prp, mempart_get_classid(mpid), memclass_pid_free);

	(void)vmm_munmap(NULL, (uintptr_t)adp->cpu.pgdir, L1TBL_SIZE, 0, mpid);
	fam_pte_asid_release(adp);
}

	
unsigned 
cpu_vmm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *paddrp, size_t *lenp) {
	pte_t			**pgdir;
	pte_t			*pde;
	pte_t			*ptep;
	unsigned		prot;
	unsigned		pg_offset;
	unsigned		pgsize;

	if(vaddr < VM_KERN_LOW_SIZE) {
		*paddrp = vaddr;
		if(lenp != NULL) *lenp = VM_KERN_LOW_SIZE - vaddr;
		return PROT_READ|PROT_WRITE|PROT_EXEC;
	}

	if(prp == NULL) {
		pgdir = get_l1pagetable();
	} else {
		pgdir = prp->memory->cpu.pgdir;
	}

	pde = PDE_ADDR(pgdir[L1PAGEIDX(vaddr)]);
	if(pde != NULL) {
		ptep = &pde[L2PAGEIDX(vaddr)];
		if(PTE_PRESENT(ptep) || (PTE_PADDR(ptep) != 0)) {
			pgsize = PTE_PGSIZE(ptep);
			pg_offset = vaddr & (pgsize - 1);
			*paddrp = PTE_PADDR(ptep) + pg_offset;
			if(lenp != NULL) *lenp = pgsize - pg_offset;
			prot = MAP_PHYS; // So we don't return PROT_NONE
			if(PTE_PRESENT(ptep)) {
				if(PTE_READABLE(ptep))   prot |= PROT_READ;
				if(PTE_EXECUTABLE(ptep)) prot |= PROT_EXEC;
				if(PTE_WRITEABLE(ptep))  prot |= PROT_WRITE;
			}
			return prot;
		}
	}
	if(vaddr >= VM_SYSPAGE_ADDR && vaddr < (VM_SYSPAGE_ADDR+__PAGESIZE)) {
		*paddrp = (uintptr_t)_syspage_ptr + (vaddr - VM_SYSPAGE_ADDR);
		if(lenp != NULL) *lenp = VM_SYSPAGE_ADDR+__PAGESIZE - vaddr;
		return PROT_READ|PROT_EXEC;
	}
	if(vaddr >= VM_CPUPAGE_ADDR && vaddr < (VM_CPUPAGE_ADDR+__PAGESIZE)) {
		*paddrp = (uintptr_t)_cpupage_ptr + (vaddr - VM_CPUPAGE_ADDR);
		if(lenp != NULL) *lenp = VM_CPUPAGE_ADDR+__PAGESIZE - vaddr;
		return PROT_READ;
	}
	return PROT_NONE;
}

__SRCVERSION("cpu_vmm.c $Rev: 169350 $");
