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

struct xfer_slot	*xfer_slots;

/*
 * Initialse msgpass xfer slots
 */
void
xfer_init()
{
	xfer_slots = _scalloc(_syspage_ptr->num_cpu * sizeof(*xfer_slots));
	if (xfer_slots == 0) {
		crash();
	}
}

static unsigned
map_slot0(unsigned addr, unsigned end)
{
	ptp_t		*ptp = (ptp_t *)ARM_V6_XFER_L1 + ARM_L1IDX(addr);
	ptp_t		*mtp = KTOL1SC(ARM_V6_XFER_BASE);

	while (addr < end) {
		*mtp++ = *ptp++;
		addr += ARM_SCSIZE;
	}
	arm_v4_dtlb_flush();
	return addr;
}

static unsigned
map_slot1(unsigned addr, unsigned base)
{
	ptp_t		*ptp = (ptp_t *)ARM_V6_XFER_L1 + ARM_L1IDX(addr);
	ptp_t		*mtp = KTOL1SC(ARM_V6_XFER_BASE + base);
	unsigned	size = ARM_V6_XFER_SIZE - base;

	while (size) {
		*mtp++ = *ptp++;
		size -= ARM_SCSIZE;
		addr += ARM_SCSIZE;
	}
	arm_v4_dtlb_flush();
	return addr;
}

static void
l1_map(ADDRESS *adp)
{
	pte_t	*pte = KTOPTEP(ARM_V6_XFER_L1);
	pte_t	l1pt = adp->cpu.l1_pte & ~ARM_PTE_V6_nG;

	pte[0] = l1pt;
	pte[1] = l1pt + __PAGESIZE;
	arm_v4_dtlb_addr(ARM_V6_XFER_L1);				// global TLB entry - no asid required
	arm_v4_dtlb_addr(ARM_V6_XFER_L1 + __PAGESIZE);	// global TLB entry - no asid required
}

static void
l1_unmap()
{
	pte_t	*pte = KTOPTEP(ARM_V6_XFER_L1);

	/*
	 * Clear page table entries and flush data TLB
	 */
	pte[0] = 0;
	pte[1] = 0;
	arm_v4_dtlb_addr(ARM_V6_XFER_L1);				// global TLB entry - no asid required
	arm_v4_dtlb_addr(ARM_V6_XFER_L1 + __PAGESIZE);	// global TLB entry - no asid required
}

int
vmm_map_xfer(PROCESS *actprp, PROCESS *prp, IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags)
{
	struct xfer_slot	*slot = &xfer_slots[RUNCPU];
	IOV					*iov = *piov;
	int					parts = *pparts;
	unsigned			bound = prp->boundry_addr;
	unsigned			diff0 = 0;
	unsigned			diff1 = 0;
	unsigned			lo0 = 0;
	unsigned			lo1 = 0;
	unsigned			hi0 = 0;
	unsigned			hi1 = 0;
	int					l1_mapped = 0;
	unsigned			addr;
	unsigned			last;
	unsigned			off;
	unsigned			size;
	int					nbytes;
	int					nparts;
	int					nparts_max;

#ifndef	NDEBUG
	if (actprp->memory == 0) {
		crash();
	}
	if (prp->memory == 0) {
		crash();
	}
#endif

	slot->prp = prp;
	slot->diff0 = 0;
	slot->size0 = 0;
	slot->diff1 = 0;
	slot->size1 = 0;

	/*
	 * Map IOV addresses if necessary
	 */
	if ((flags & MAPADDR_FLAGS_IOVKERNEL) == 0) {
		last = (uintptr_t)iov + parts * sizeof(IOV) - 1;
		if ((uintptr_t)iov > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)iov, last, bound))) {
			return -1;
		}
		if (V6_USER_SPACE(iov)) {
			/*
			 * Map the iov list
			 */
			if (l1_mapped == 0) {
				l1_map(prp->memory);
				l1_mapped = 1;
			}
			lo0 = (unsigned)iov & ~ARM_SCMASK;
			hi0 = map_slot0(lo0, last);
			slot->diff0 = diff0 = ARM_V6_XFER_BASE - lo0;
			slot->size0 = hi0 - lo0;

			iov = (IOV *)((unsigned)iov + diff0);
		}
	}

	/*
	 * Check whole IOV list is valid
	 */
	addr = ((uintptr_t)iov) & ~PGMASK;
	last = ((uintptr_t)(iov + (uint32_t)parts) - 1) & ~PGMASK;
	if (addr > last) {
		nbytes = -1;
		goto out;
	}
	while (addr <= last) {
		if (xfer_memprobe((void *)addr) != 0) {
			nbytes = -1;
			goto out;
		}
		addr += __PAGESIZE;
	}

	/*
	 * Skip to supplied offset
	 */
	off = *poff;
	while (off >= (size = GETIOVLEN(iov))) {
		off -= size;
		if (--parts == 0) {
			nbytes = 0;		// no more parts
			goto out;
		}
		iov++;
	}
	addr = (unsigned)GETIOVBASE(iov) + off;
	size = (unsigned)GETIOVLEN(iov)  - off;
	off = 0;

	/*
	 * Loop over remaining IOVs and adjust to mapped addresses
	 */
	nbytes = 0;
	nparts = 0;
	nparts_max = *pnparts;
	while (nparts < nparts_max) {
		unsigned	map_size;
		unsigned	map_addr;
		unsigned	len;

		if (size) {
			/*
			 * Check addresses are valid
			 */
			last = addr + size - 1;
			if (addr > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY(addr, last, bound))) {
				nbytes = -1;
				goto out;
			}
			if (!V6_USER_SPACE(addr)) {
				/*
				 * Kernel address - no mapping required
				 */
				map_size = size;
				map_addr = addr;
			}
			else if (addr >= lo0 && addr < hi0) {
				/*
				 * Contained in first mapping
				 */
				map_addr = addr + diff0;
				map_size = hi0 - addr;
			}
			else if (addr >= lo1 && addr < hi1) {
				/*
				 * Contained in second mapping
				 */
				map_addr = addr + diff1;
				map_size = hi1 - addr;
			}
			else if (hi1 == 0) {
				/*
				 * Create second set of mappings
				 */
				if (l1_mapped == 0) {
					l1_map(prp->memory);
					l1_mapped = 1;
				}
				lo1 = addr & ~ARM_SCMASK;
				hi1 = map_slot1(lo1, slot->size0);
				slot->diff1 = diff1 = ARM_V6_XFER_BASE + slot->size0 - lo1;
				slot->size1 = ARM_V6_XFER_SIZE - slot->size0;

				map_addr = addr + diff1;
				map_size = hi1 - addr;
			}
			else {
				/*
				 * Can't map the iov data
				 */
				break;
			}

			len = min(size, map_size);
			SETIOV(niov, map_addr, len);
			niov++;
			nparts++;
			nbytes += len;
			if (size > map_size) {
				/*
				 * Could only map part of the iov
				 */
				off = (addr - (unsigned)GETIOVBASE(iov)) + len;
				break;
			}
		}
		if (--parts == 0) {
			break;
		}
		iov++;
		addr = (unsigned)GETIOVBASE(iov);
		size = GETIOVLEN(iov);
	}

	/*
	 * Adjust caller's iov list to the end of the area we just mapped
	 */
	*piov = (IOV *)((unsigned)iov - diff0);
	*pparts = parts;
	*poff = off;
	*pnparts = nparts;

out:
	if (l1_mapped) {
		l1_unmap();
	}
	return nbytes;
}

__SRCVERSION("vmm_map_xfer.c $Rev: 198927 $");
