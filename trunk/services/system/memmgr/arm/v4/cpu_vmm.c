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

#define	MAX_ASID	64

static ADDRESS	*asid_map[MAX_ASID];

#define	ARM_GBL_POOL_SIZE	__PAGESIZE
static struct arm_gbl_map	*gbl_free;
static pthread_mutex_t		gbl_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;


void
vx_capability_check(void)
{
	if (SYSPAGE_ENTRY(cpuinfo)->flags & ARM_CPU_FLAG_V6) {
		kprintf("This kernel does not support ARMv6 MMU\n");
		crash();
	}
}

// FIX ME - no prp coming in right now. I think this is like a SOUL list.Need
//			same controls. Why can't these ARM global mappings be an arch specific SOUL type?
static struct arm_gbl_map *
arm_gbl_alloc()
{
	struct arm_gbl_map	*mp;

	pthread_mutex_lock(&gbl_alloc_mutex);
	if (gbl_free == 0) {
		int		i;

		/*
		 * Allocate a chunk of arm_gbl_map structs and link into freelist
		 */
		gbl_free = mp = _smalloc(ARM_GBL_POOL_SIZE);
		for (i = ARM_GBL_POOL_SIZE / sizeof(*mp); --i; mp++) {
			mp->next = mp + 1;
		}
		mp->next = 0;
	}

	mp = gbl_free;
	if (mp) {
		gbl_free = mp->next;
	}
	pthread_mutex_unlock(&gbl_alloc_mutex);
	return mp;
}

static void
arm_gbl_free(struct arm_gbl_map *mp)
{
	pthread_mutex_lock(&gbl_alloc_mutex);
	mp->next = gbl_free;
	gbl_free = mp;
	pthread_mutex_unlock(&gbl_alloc_mutex);
}

/*
 * Update the domain field in the L1 entries for protected global mappings.
 */
void
arm_gbl_update(ADDRESS *adp, int all)
{
	struct arm_gbl_map	*mp;
	int8_t				domain = adp->cpu.domain;

	for (mp = adp->cpu.gbl_map; mp; mp = mp->next) {
		/*
		 * all is set if the domain changed, so update all mappings.
		 * Otherwise, only update SHMCTL_GLOBAL mappings
		 */
		if (all || (mp->idx & ARM_GBL_MAP_GLOBAL) != 0) {
			ptp_t	*ptp = L1_table + ARM_GBL_MAP_IDX(mp);
			ptp_t	ptmp;

			if (ptp == 0) {
				crash();
			}
			ptmp = *ptp;
			PTP_DOMAIN_CLR(ptmp);
			PTP_DOMAIN_SET(ptmp, domain);
			*ptp = ptmp;
		}
	}
}

/*
 * Called by mmap code to record a protected global mapping
 */
int
cpu_gbl_mmap(ADDRESS *adp, uintptr_t start, uintptr_t end, unsigned flags)
{
	struct arm_gbl_map	**pp;
	struct arm_gbl_map	*mp;
	struct arm_gbl_map	*np;
	unsigned short		idx;

	if (!ARM_GBL_PROTECTED(flags)) {
		return 0;
	}

	/*
	 * Search the gbl_map list for entry containing start:
	 * - mp will be set to entry for vaddr <= start
	 * - np will be set to entry following the entry for vaddr <= start
	 *
	 * If necessary we insert entries for [start,end) between mp and np.
	 */
	idx = ARM_L1IDX(start);
	for (pp = &adp->cpu.gbl_map; (np = *pp) && ARM_GBL_MAP_IDX(np) < idx; pp = &np->next)
		;
	mp = *pp;

	while (start < end) {
		unsigned npages;

		/*
		 * Calculate the number of pages required in this 1MB section
		 */
		npages = end + 1 - start;
		if (npages > ARM_SCSIZE) {
			npages = ARM_SCSIZE - (start & ARM_SCMASK);
		}
		npages >>= 12;

		if (mp && ARM_GBL_MAP_IDX(mp) == idx) {
			mp->cnt += npages;
		}
		else {
			if ((mp = arm_gbl_alloc()) == 0) {
				return -1;
			}
			mp->idx = idx;
			mp->cnt = npages;
			mp->next = np;
			if (flags & SHMCTL_GLOBAL) {
				mp->idx |= ARM_GBL_MAP_GLOBAL;
				adp->cpu.gbl_swtch++;
			}
			*pp = mp;
			pp = &mp->next;
		}
		start += npages << 12;
		idx = ARM_L1IDX(start);
	}
	return 0;
}

/*
 * Called by munmap code to remove a protected global mapping
 */
void
cpu_gbl_unmap(ADDRESS *adp, uintptr_t start, uintptr_t end, unsigned flags)
{
	struct arm_gbl_map	**pp;
	struct arm_gbl_map	*mp;
	unsigned short		idx;

	if (!ARM_GBL_PROTECTED(flags)) {
		return;
	}

	/*
	 * Search list for entry containing start
	 */
	idx = ARM_L1IDX(start);
	for (pp = &adp->cpu.gbl_map; (mp = *pp) && ARM_GBL_MAP_IDX(mp) < idx; pp = &mp->next)
		;
	if (mp == 0 || ARM_GBL_MAP_IDX(mp) != idx) {
#ifndef	NDEBUG
		/*
		 * Didn't find an entry corresponding to the start address.
		 * We expect to be called only for valid mapped regions.
		 */
		crash();
#endif
		return;
	}

	while (start < end) {
		unsigned npages;

		/*
		 * Calculate the number of pages required in this 1MB section
		 */
		npages = end + 1 - start;
		if (npages > ARM_SCSIZE) {
			npages = ARM_SCSIZE - (start & ARM_SCMASK);
		}
		npages >>= 12;

		if (ARM_GBL_MAP_IDX(mp) == idx && (mp->cnt -= npages) == 0) {
			*pp = mp->next;
			if (mp->idx & ARM_GBL_MAP_GLOBAL) {
				adp->cpu.gbl_swtch--;
				if (adp->cpu.gbl_swtch < 0) {
					crash();
				}
			}
			arm_gbl_free(mp);
			if ((mp = *pp) == 0) {
				/*
				 * No more global mappings in this address space.
				 */
				break;
			}
		}
		start += npages << 12;
		idx = ARM_L1IDX(start);
	}
}

void
cpu_cache_control(ADDRESS *adp, void *addr, size_t len, unsigned flags)
{
	uintptr_t	mva = (uintptr_t)addr;

	if (mva < USER_SIZE) {
		mva |= MVA_BASE(adp->cpu.asid);
	}
	CacheControl((void *)mva, len, flags);
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

	/*
	 * Check if fault is accessing another address space via msgpass
	 */
	if ((info->sigcode & SIGCODE_INXFER) && 
		xfer_prp &&
		vaddr >= USER_SIZE &&
		vaddr >= xfer_diff &&
		vaddr <  xfer_diff + USER_SIZE) {
		prp = xfer_prp;
		vaddr -= xfer_diff;
	}

	/*
	 * Check if fault cannot be fixed up by fault_pulse()
	 * - read fault on a valid translation
	 * - write fault on a valid translation with write access
	 */
	mva = vaddr & ~PGMASK;
	if (mva < USER_SIZE) {
		mva |= MVA_BASE(prp->memory->cpu.asid);
	}
	if (*VTOPDIR(mva) & ARM_PTE_VALID) {
		pte = *VTOPTEP(mva);
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
	int		asid;

	// domain will be allocated by vmm_aspace()
	adp->cpu.domain = 1;

	/*
	 * WARNING: we are currently called with kernel locked so no other
	 *          locks are required to manipulate asid_map[]
	 */
	for (asid = 1; asid < MAX_ASID; asid++) {
		if (asid_map[asid] == 0) {
			break;
		}
	}
	if (asid == MAX_ASID) {
		return EAGAIN;
	}
	asid_map[asid] = adp;
	adp->cpu.asid = asid;
	adp->cpu.gbl_swtch = 0;
	adp->cpu.gbl_map   = 0;

	//For use by the kernel dumper...
	adp->cpu.pgdir = L1_table;
	return EOK;
}

void
cpu_vmm_mdestroy(PROCESS *prp)
{
	ADDRESS 	*adp = prp->memory;
	unsigned	va;
	unsigned	sz;
	part_id_t mpid = mempart_getid(prp, sys_memclass_id);
	memsize_t memclass_pid_free = 0;

	/*
	 * Free page tables
	 */
	for (va = MVA_BASE(adp->cpu.asid), sz = USER_SIZE; sz; va += PDE_SPAN, sz -= PDE_SPAN) {
		pte_t	*pte = VTOPDIR(va);

		if (*pte) {
			ptp_t	*ptp = VTOL1PT(va);

			// free page table page
			pa_free_paddr(*pte & ~PGMASK, __PAGESIZE, MEMPART_DECR(mpid, __PAGESIZE));
			memclass_pid_free += __PAGESIZE;

			// unmap from ARM_PTE_MAP
			*pte = 0;

			// unmap from L1 page table
			*ptp++ = 0;
			*ptp++ = 0;
			*ptp++ = 0;
			*ptp = 0;

			// flush TLB mapping for ARM_PTE_MAP mapping
			arm_v4_dtlb_addr((unsigned)VTOPTP(va));
		}
	}
	MEMCLASS_PID_FREE(prp, mempart_get_classid(mpid), memclass_pid_free);

	/*
	 * Release our ASID (ProcessID register mapping).
	 *
	 * WARNING: we are currently called with kernel locked so no other
	 *          locks are required to manipulate asid_map[]
	 */
	if (adp->cpu.asid) {
		asid_map[adp->cpu.asid] = 0;
	}

	/*
	 * Release our domain.
	 */
	if (adp->cpu.domain > 1) {
		domain_free(adp);
	}
}


unsigned
cpu_vmm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *p, size_t *lenp)
{
	ADDRESS		*adp = prp ? prp->memory : 0;
	unsigned	pte;
	unsigned	pg_size;
	unsigned	pg_offset;
	unsigned	prot;

	if (adp && vaddr < USER_SIZE) {
		vaddr |= MVA_BASE(adp->cpu.asid);
	}

	prot = MAP_PHYS; // so PROT_NONE doesn't come back

	/*
	 * Check for section mapping
	 */
	if (((pte = *VTOL1SC(vaddr)) & 0x3) == (ARM_PTP_SC & 0x3)) {
		pg_offset = vaddr & ARM_SCMASK;
		*p = (pte & ~ARM_SCMASK) | pg_offset;

		if(lenp != NULL) {
			*lenp = ARM_SCSIZE - pg_offset;
		}

		prot |= PROT_READ | PROT_EXEC;

		if ((pte & ARM_PTP_AP_RW) != 0) {
			prot |= PROT_WRITE;
		}
		if ((pte & ARM_PTP_C) == 0) {
			prot |= PROT_NOCACHE;
		}
	}
	else {
		if (*VTOPDIR(vaddr) == 0 || !((pte = *VTOPTEP(vaddr)) & ARM_PTE_VALID)) {
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

		if ((pte & ARM_XSP_PROT(ARM_PTE_RW)) == ARM_XSP_PROT(ARM_PTE_RW)) {
			prot |= PROT_WRITE;
		}
		if ((pte & ARM_PTE_C) == 0) {
			prot |= PROT_NOCACHE;
		}
	}
	return prot;
}

__SRCVERSION("cpu_vmm.c $Rev: 173095 $");
