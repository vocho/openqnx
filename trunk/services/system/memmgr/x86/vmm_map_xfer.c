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


//RUSH3: There are probably pieces of this code that can be pulled
//RUSH3: into support routines placed in the CPU-independent directory

int 
vmm_map_xfer(PROCESS *actprp, PROCESS *prp,  IOV **piov, int *pparts, int *poff, IOV *pniov, int *pnparts, unsigned flags) {
	unsigned	base;
	uintptr_t	last;
	IOV 		*iov, *niov;
	unsigned 	parts, off, addr, size, nparts, nparts_max, bytes, bound, len;
	unsigned	diff0, lo0, hi0, diff1, lo1, hi1;
	pxe_t		*l1vaddr, *pl1, *pl1m;
	uintptr_t	addr_map;
	struct xfer_slots	*slot;
	ADDRESS		*madp;
	unsigned	pde_size = 1 << pd_bits;

#ifndef NDEBUG
	ADDRESS							*adp;

	if(!prp || !(adp = prp->memory) || !(l1vaddr = adp->cpu.pgdir)) {
		return -1;
	}
#else
	l1vaddr = prp->memory->cpu.pgdir;
#endif

	// Get our cpu's slot structure
	slot = &xfer_slot[RUNCPU];
	*((volatile uintptr_t *)&slot->base0) = 0;

	lo0 = lo1 = diff0 = diff1 = 0;
	// check and map IOV address
	hi0 = 0;
	madp = actprp->memory;
	iov = (IOV*)*piov;
	parts = *pparts;
	bound = prp->boundry_addr;
	if((flags & MAPADDR_FLAGS_IOVKERNEL) == 0) {
#ifndef NDEBUG
		// boundry check
		last = (uintptr_t)iov + parts*sizeof(IOV)  - 1;
		if((uintptr_t)iov > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)iov, last, bound))) {
				return -1;
		}
#endif

		if((uintptr_t)iov < CPU_USER_VADDR_END) {

			// usr address space
			// Find the next slot available
			base = slot->addr + (pde_size << 1);
			if(base >= slot->last) {
				base = slot->first;
			}

			*((volatile uintptr_t*)&slot->addr) = *((volatile uintptr_t *)&slot->base0) = base;

			pl1 = GENERIC_VTOPDIRP(l1vaddr, (uintptr_t)iov);
			if(madp) {
				pl1m = GENERIC_VTOPDIRP(madp->cpu.pgdir, (uintptr_t)base);
				if(pae_enabled) {
					pl1m->pxe64 = pl1->pxe64  & ~(uint64_t)X86_PTE_USER;
					pl1m = PXE_ADD(pl1m, sizeof(pl1m->pxe64));
					pl1 = PXE_ADD(pl1, sizeof(pl1->pxe64));
					pl1m->pxe64 = pl1->pxe64  & ~(uint64_t)X86_PTE_USER;
				} else {
					pl1m->pxe32 = pl1->pxe32  & ~X86_PTE_USER;
					pl1m = PXE_ADD(pl1m, sizeof(pl1m->pxe32));
					pl1 = PXE_ADD(pl1, sizeof(pl1->pxe32));
					pl1m->pxe32 = pl1->pxe32  & ~X86_PTE_USER;
				}
			} else {
crash();
			}
			flushtlb();
			lo0 = (unsigned)iov & ~(pde_size - 1);
			hi0 = lo0 + (pde_size<<1);
			diff0 = base - lo0;
			iov = (void*)((unsigned)iov + diff0);
			slot->prp = prp;
			*((volatile ptrdiff_t*)&slot->diff0) = diff0;
		}

	}

// check if the whole iov are all valid. If not, return error.
	addr = ((uintptr_t)iov) & ~(__PAGESIZE - 1);
	last = ((uintptr_t)(iov + (uint32_t)parts) - 1) & ~(__PAGESIZE - 1);
	if(addr > last) {
		return -1;
	}
	while(addr <= last) {
		if(xfer_memprobe((void*)addr) != 0) {
			return -1;
		}
		addr += __PAGESIZE;
	}

	// skip offsets
	off = *poff;
	while(off >= (len = GETIOVLEN(iov))) {
		if(--parts == 0) { 	/* No more parts. */
			return 0;
		}
		off -= len;
		iov ++;
	}
	addr = (unsigned)GETIOVBASE(iov) + off;
	size = len - off;

	// boundry check
	last = (uintptr_t)addr + size - 1;
	if(addr > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)addr, last, bound))) {
			if(size != 0) {
				return -1;
			}
	}

	// mapping loop
	nparts = 1;
	niov = (IOV*)pniov;
	nparts_max = *pnparts;
	bytes = 0;
	hi1 = 0;
	
	do {
		if((uintptr_t)addr > CPU_USER_VADDR_END) {
			// system process and kernel address space
			len = size;
			addr_map = addr;
		} else {
			if((addr < hi0) && (addr >= lo0)) {
				// within the first mapping area
				len = hi0 - addr;
				addr_map = addr + diff0;
			} else {
				if((addr < hi1) && (addr >= lo1)) {
					// within the second slot
					len = hi1 - addr;
					addr_map = addr + diff1;
				} else {
					if(hi1 == 0) {
						// we could allocate the second mapping area here
						// Find the next slot available
						base = slot->addr + (pde_size << 1);
						if(base >= slot->last) {
							base = slot->first;
						}

						*((volatile uintptr_t *)&slot->addr) = base;

						pl1 = GENERIC_VTOPDIRP(l1vaddr, addr);
						pl1m = GENERIC_VTOPDIRP(madp->cpu.pgdir, base);
						if(pae_enabled) {
							pl1m->pxe64 = pl1->pxe64  & ~(uint64_t)X86_PTE_USER;
							pl1m = PXE_ADD(pl1m, sizeof(pl1m->pxe64));
							pl1 = PXE_ADD((uintptr_t)pl1, sizeof(pl1->pxe64));
							pl1m->pxe64 = pl1->pxe64  & ~(uint64_t)X86_PTE_USER;
						} else {
							pl1m->pxe32 = pl1->pxe32  & ~X86_PTE_USER;
							pl1m = PXE_ADD(pl1m, sizeof(pl1m->pxe32));
							pl1 = PXE_ADD((uintptr_t)pl1, sizeof(pl1->pxe32));
							pl1m->pxe32 = pl1->pxe32  & ~X86_PTE_USER;
						}
						flushtlb();
						slot->prp = prp;
						lo1 = addr & ~(pde_size - 1);
						hi1 = lo1 + (pde_size << 1);
						diff1 = base - lo1;

						slot->diff = diff1;

						len = hi1 - addr;
						addr_map = addr + diff1;
					} else {
						// no mapping space is available, return
						break;
					}
				}
			}
		}	

		// mapped area is bigger?
		if(len >= size) {
			// one source iov is ended
			SETIOV(niov, addr_map, size);
			bytes += size;
			do {
				if(--parts == 0) {
					// finish, no source iov left. reutrn home
					*pnparts = nparts;
					*pparts = parts;
					return bytes;
				}
				iov ++;
				addr = (unsigned)GETIOVBASE(iov);
				size = GETIOVLEN(iov);
			} while (size == 0);
			// boundry check
			last = (uintptr_t)addr + size - 1;
			if(addr > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)addr, last, bound))) {
				if(size != 0) {
					return -1;
				}
			}
		} else {
			size -= len;
			bytes += len;
			addr += len;
			SETIOV(niov, addr_map, len);
		}

		niov ++;
	} while(++nparts <= nparts_max);

	/* still have something left in src iov chain */
	*pnparts = --nparts;
	*pparts = parts;
	*poff = GETIOVLEN(iov) - size;
	if(((flags & MAPADDR_FLAGS_IOVKERNEL) == 0) && ((unsigned)iov >= MAP_BASE) && ((unsigned)iov < (MAP_BASE+MAP_SIZE))) {
		*piov = (void *)((unsigned)iov - diff0);
	} else {
		*piov = iov;
	}

	return bytes;
}

__SRCVERSION("vmm_map_xfer.c $Rev: 153052 $");
