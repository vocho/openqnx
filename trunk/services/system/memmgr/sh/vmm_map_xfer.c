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
 * Allow map_xfer to cover the following:
 * - up to 8MB to cover IOV list (max 4MB of iovs + alignment considerations)
 * - up to 8MB to cover data that is not in the first 8MB mapping
 */
#define	XFER_SIZE	MEG(8)
#define	XFER_BASE0	VM_MSG_XFER_START
#define	XFER_BASE1	(XFER_BASE0 + XFER_SIZE)

/*
 * Map at least the supplied IOV list from map->prp into prp's address space
 */
static inline unsigned
map_slot0(PROCESS *prp, struct xfer_map *map, unsigned addr, unsigned end)
{
	ADDRESS		*dadp = prp->memory;
	ADDRESS		*sadp = map->prp->memory;
	uint32_t	**dpde = &dadp->cpu.pgdir[L1PAGEIDX(XFER_BASE0)];
	uint32_t	**spde = &sadp->cpu.pgdir[L1PAGEIDX(addr)];

	map->diff0 = XFER_BASE0 - addr;
	map->size0 = XFER_SIZE;

	dpde[0] = spde[0];
	dpde[1] = spde[1];

	/*
	 * Flush any old TLB entry that covered the VM_MSG_XFER_START range.
	 *
	 * WARNING: we rely on ker/sh/vm4.s to use only TLB entry 0 for msg xfer
	 */
	tlb_flush_entry(prp->memory, 0);
	return addr + XFER_SIZE;
}

/*
 * Map a chunk of message data that wouldn't fit in the map's first slot
 */
static inline unsigned
map_slot1(PROCESS *prp, struct xfer_map *map, unsigned addr)
{
	ADDRESS		*dadp = prp->memory;
	ADDRESS		*sadp = map->prp->memory;
	uint32_t	**dpde = &dadp->cpu.pgdir[L1PAGEIDX(XFER_BASE1)];
	uint32_t	**spde = &sadp->cpu.pgdir[L1PAGEIDX(addr)];

	map->diff1 = XFER_BASE1 - addr;
	map->size1 = XFER_SIZE;

	dpde[0] = spde[0];
	dpde[1] = spde[1];

	/*
	 * Flush any old TLB entry that covered the VM_MSG_XFER_START range.
	 *
	 * WARNING: we rely on ker/sh/vm4.s to use only TLB entry 0 for msg xfer
	 */
	if (map->size0 == 0) {
		tlb_flush_entry(prp->memory, 0);
	}
	return addr + XFER_SIZE;
}

int
vmm_map_xfer(PROCESS *actprp, PROCESS *prp, IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags)
{
	struct xfer_map	*map = &xfer_map[RUNCPU];
	IOV				*iov = *piov;
	int				parts = *pparts;
	unsigned		bound = prp->boundry_addr;
	unsigned		lo0 = 0;
	unsigned		lo1 = 0;
	unsigned		hi0 = 0;
	unsigned		hi1 = 0;
	unsigned		diff0 = 0;
	unsigned		diff1 = 0;
	unsigned		addr;
	unsigned		last;
	unsigned		off;
	unsigned		size;
	int				nbytes;
	int				nparts;
	int				nparts_max;

#ifndef	NDEBUG
	if (actprp->memory == 0) {
		crash();
	}
	if (prp->memory == 0) {
		crash();
	}
#endif

	map->prp = prp;
	map->diff0 = 0;
	map->size0 = 0;
	map->diff1 = 0;
	map->size1 = 0;

	/*
	 * Map IOV addresses if necessary
	 */
	if ((flags & MAPADDR_FLAGS_IOVKERNEL) == 0) {
		addr = (uintptr_t)iov;
		last = addr + parts * sizeof(IOV) - 1;
		if (addr > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY(addr, last, bound))) {
			return -1;
		}
		if (addr < VM_USER_SPACE_BOUNDRY) {
			/*
			 * Map the iov list
			 */
			lo0 = (unsigned)iov & ~(MEG(4)-1);
			hi0 = map_slot0(actprp, map, lo0, last);
			diff0 = map->diff0;
			iov = (IOV *)(addr + diff0);
		}
		else if (addr > VM_KERN_SPACE_BOUNDRY) {
			/*
			 * Not a valid system address
			 */
			return -1;
		}
	}

	/*
	 * Check whole IOV list is valid
	 */
	addr = ((uintptr_t)iov) & ~(__PAGESIZE-1);
	last = ((uintptr_t)(iov + (uint32_t)parts) - 1) & ~(__PAGESIZE-1);
	if (addr > last) {
		return -1;
	}
	while (addr <= last) {
		if (xfer_memprobe((void *)addr) != 0) {
			return -1;
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
			*pnparts = 0;
			return 0;		// no more parts
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
				return -1;
			}
			if (addr >= VM_USER_SPACE_BOUNDRY) {
				/*
				 * System address - no mapping required
				 */
				map_size = size;
				map_addr = addr;
			}
			else { 
#if 0
				if (hi0 == 0) {
					/*
					 * Create initial mapping
					 */
					lo0 = addr & ~(MEG(4)-1);
					hi0 = map_slot0(actprp, map, lo0, last);
					diff0 = map->diff0;
				}
#endif
				if (addr >= lo0 && addr < hi0) {
					/*
					 * Contained in first mapping
					 */
					map_addr = addr + diff0;
					map_size = hi0 - addr;
				}
				else {
					if (hi1 == 0) {
						/*
						 * Create second set of mappings
						 */
						lo1 = addr & ~(MEG(4)-1);
						hi1 = map_slot1(actprp, map, lo1);
						diff1 = map->diff1;
					}
					if (addr >= lo1 && addr < hi1) {
						/*
						 * Contained in second mapping
						 */
						map_addr = addr + diff1;
						map_size = hi1 - addr;
					}
					else {
						/*
						 * Can't map the iov data
						 */
						break;
					}
				}
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
				off = (addr - (unsigned)GETIOVBASE(iov)) + nbytes;
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

	return nbytes;
}

__SRCVERSION("vmm_map_xfer.c $Rev: 153052 $");
