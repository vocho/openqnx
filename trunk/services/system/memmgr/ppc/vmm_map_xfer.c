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


// ***************************************************************
// ***************************************************************
// ** 
// ** NOTE that this file is OVERRIDDEN for procnto-600. See
// ** "memmgr/ppc/600-900/vmm_map_xfer.c" for the code used in that case.
// **
// ***************************************************************
// ***************************************************************

#define VM_MSG_SLOT_SIZE	(1 << L1_SHIFT)

//RUSH3: We should be stuffing L1 page table pointers into the active
//RUSH3: address space the same way we do on the X86 - that will make
//RUSH3: message passing faster and allow us to remove code in cpu_vmm_fault().

static ptrdiff_t
vmm_map_addr_old(PROCESS *actprp, PROCESS *prp, void *addr, size_t size, void **plo, void **phi, int flags) {
	uint8_t							*base;
	ADDRESS							*adp;
	uintptr_t						trans_base;
	uintptr_t						last;

	last = (uintptr_t)addr + size - 1;
	if(prp) {
		if((uintptr_t)addr > last || (!WITHIN_BOUNDRY((uintptr_t)addr, last, prp->boundry_addr) && !(flags & MAPADDR_FLAGS_SYSPRP))) {
			if(size != 0) {
				return -1;
			}
		}
	}

	if((uintptr_t)addr < VM_KERN_LOW_SIZE) {
		*plo = (void *)VM_KERN_SPACE_BOUNDRY;
		*phi = (void *)(VM_KERN_SPACE_BOUNDRY + VM_KERN_LOW_SIZE-1);
		return 0;
	}
	if((uintptr_t)addr >= VM_CPUPAGE_ADDR) {
		//Handle mappings for CPU, system pages (OK to be over-generous,
		//we'll just fault if there's no mapping installed).
		*plo = (void *)VM_CPUPAGE_ADDR;
		*phi = (void *)(~0);
		return 0;
	}

	if(!prp || !(adp = prp->memory)) {
		return -1;
	}

	/* need to check: protection, continuous, cacheability */
	base = (void *)(ADDR_PAGE((uintptr_t)addr));

	xfer_prp = prp;
	*plo = base;

	last = (uintptr_t)base + VM_MSG_SLOT_SIZE;
	if(last < (uintptr_t)base) last = ADDR_PAGE(~0);

	*phi = (void *)(last - 1);
	trans_base = VM_MSG_XFER_START | (((xfer_lastaddr[xfer_rotor] + VM_MSG_SLOT_SIZE) & ~(VM_MSG_SLOT_SIZE-1)) & (VM_MSG_XFER_START - 1));

	fam_pte_flush_all();

	xfer_diff[0] = trans_base - (uintptr_t)base;
	return xfer_diff[0];
}


// General one which could get rid of 4M page limit for msg xfer
static int
vmm_map_addr_general(PROCESS *actprp, PROCESS *prp,  IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags) {
	void *hi, *lo;
	IOV *iov, *iov_o;
	unsigned n, parts, parts_o, off, nparts, nparts_max;
	unsigned nbytes;
	uintptr_t last, base, len;
	ptrdiff_t diff;
	IOV	iovlist[32];

	// map in iov
	iov_o = *piov;
	parts_o = *pparts;
	if(!(flags & MAPADDR_FLAGS_IOVKERNEL)) {
#ifndef NDEBUG
		// boundry check
		last = (uintptr_t)iov_o + parts_o*sizeof(IOV)  - 1;
		if((uintptr_t)iov_o > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)iov_o, last, prp->boundry_addr))) {
			return -1;
		}
#endif
		diff = vmm_map_addr_old(actprp, prp, iov_o, parts_o * sizeof *iov_o, &lo, &hi, flags);
		if(diff == -1) {
			return(-1);
		}
	} else {
		hi = lo = 0;
		diff = 0;
	}

	// check iov readability

	base = ADDR_PAGE((uintptr_t)iov_o);
	last = ADDR_PAGE((uintptr_t)(iov_o +  (uintptr_t)parts_o));

	while(base <= last) {
		if(xfer_memprobe((void*)(base + diff)) !=0) return -1;
		base += __PAGESIZE;
	}

	// skip over offsets
	nparts = 0;
	off = *poff;
	while(off >= (len=GETIOVLEN((IOV*)((uintptr_t)iov_o + diff))) ) {
		off -= len;
		if(--parts_o == 0) { 	/* No more parts. */
			*pnparts = nparts;
			return 0;
		}
		++iov_o;
	}

	n = min(sizeof iovlist / sizeof *iovlist, parts_o);
	memcpy(iovlist, (IOV *)((uintptr_t)iov_o + diff), n * sizeof *iov);
	iov = iovlist;

	base = (uintptr_t)GETIOVBASE(iov);
	len = GETIOVLEN(iov);
	if(off) {
		base += off;
		len -= off;
		off = 0;
	}

	nbytes = 0;
	nparts_max = *pnparts;
	parts = 0;
	while(nparts < nparts_max) {
 		/* Make sure address is within range for process */
		if(len) {
			last = base + len - 1;
			if(base > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY(base, last, prp->boundry_addr))) {
				return -1;
			}
   	
			if(base < (uintptr_t)lo || last > (uintptr_t)hi) {
				if(nparts) {
					break;
				}
				diff = vmm_map_addr_old(actprp, prp,
						(void*)base, len, &lo, &hi, flags);
				if(diff == -1) {
					return -1;
				}
			}

			if(last > (uintptr_t) hi) {
				// larger than one map
				// as it is the first iov to map in ...
				len -= (last - (uintptr_t)hi);
				SETIOV(niov, (IOV *)((uintptr_t)base + diff), len);
				nparts ++;
				off = len + (base - (uintptr_t) GETIOVBASE(iov));
				nbytes += len;
				break;
			}

			SETIOV(niov, (IOV *)((uintptr_t)base + diff), len);
			nbytes += len;
			nparts ++;
			niov ++;
		}
		parts++;
		if(parts >= n) break;
		iov++;
		base = (uintptr_t)GETIOVBASE(iov);
		len = (uintptr_t)GETIOVLEN(iov);
	}

	*piov = iov_o + parts;
	*pparts = parts_o - parts;
	*pnparts = nparts;
	*poff = off;

	return nbytes;
}

//RUSH1: Have to use vmm_map_addr_general() if 'pte' is little endian mapping
//RUSH1: (if we're big endian).
#define TRANSLATE_IOV_ADDR(iov,iov_phy)															\
	{																							\
	pte_t	*pde;																				\
	pte_t	*ptep;																				\
	if((pde = PDE_ADDR(pgdir[L1PAGEIDX(iov)])) == 0												\
		 || !PTE_PRESENT(ptep = &pde[L2PAGEIDX(iov)]) || !PTE_READABLE(ptep)) {					\
		return -1;																				\
	}																							\
	paddr = PTE_PADDR(ptep) | ((uintptr_t)iov & (PTE_PGSIZE(ptep)-1));							\
	if(!PTE_CACHEABLE(ptep) || (paddr >= VM_KERN_LOW_SIZE)) {									\
		return vmm_map_addr_general(actprp, prp,  piov, pparts, poff, niov, pnparts, flags);	\
	}																							\
	iov_phy =(IOV *)(uintptr_t)paddr; 															\
	if(xfer_memprobe(iov_phy) != 0) return -1;													\
	}

int
vmm_map_xfer(PROCESS *actprp, PROCESS *prp,  IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags) {
	unsigned	len;
	IOV 		*iov_phy, *iov;
	IOV			*orig_niov = niov;
	uintptr_t 	addr;
	unsigned	parts, size, nparts, nparts_max, bytes, bound;
	paddr_t		addr_phy;
	pte_t		**pgdir;
	uintptr_t	last, off;
	paddr_t		paddr;

#ifndef NDEBUG
	ADDRESS		*adp;

	if(!prp || !(adp = prp->memory) || !(pgdir = adp->cpu.pgdir)) {
		return -1;
	}
#else
	pgdir = prp->memory->cpu.pgdir;
#endif

	// check and map IOV address
	iov = (IOV*)*piov;
	parts = *pparts;
	bound = prp->boundry_addr;
	if(!(flags & MAPADDR_FLAGS_IOVKERNEL)) {
#ifndef NDEBUG
			// boundry check
			last = (uintptr_t)iov + parts*sizeof(IOV)  - 1;
			if((uintptr_t)iov > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)iov, last, prp->boundry_addr))) {
				return -1;
			}
#endif
		if((uintptr_t)iov < VM_USER_SPACE_BOUNDRY) {
			// system process and kernel address space
			iov_phy = iov;
			flags |= MAPADDR_FLAGS_IOVKERNEL;
		} else {
			if (((uintptr_t)iov & (sizeof(_Int32t) - 1)) ||
				((((uintptr_t)iov + 4) & (__PAGESIZE - 1)) == 0))	// if iov crosses a page boundary
			{
				/* not int32 aligned, use the old interface */
				return vmm_map_addr_general(actprp, prp,  piov, pparts, poff, niov, pnparts, flags);
			}	
			TRANSLATE_IOV_ADDR(iov,iov_phy)
		}
	} else {
		iov_phy = iov;
	}

	// skip offsets
	off = *poff;
	addr = 0;
	while(off >= (size = GETIOVLEN(iov_phy))) {
		off -= size;
		if(--parts == 0) { 	/* No more parts. */
			*pnparts = parts;
			return 0;
		}
		++iov_phy;
		addr = 0;	
		if((ADDR_OFFSET((uintptr_t)iov_phy + sizeof(_Int32t)) <= sizeof(_Int32t)) &&
			!(flags & MAPADDR_FLAGS_IOVKERNEL)) {
			int nalign = ADDR_OFFSET(iov_phy);

			if(nalign) {
				// not eight bytes aligned, get addr first
				addr = (uintptr_t)GETIOVBASE(iov_phy);
			}

			// adjust iov_phy
			iov = (IOV *)ADDR_PAGE((uintptr_t)iov + __PAGESIZE);
			TRANSLATE_IOV_ADDR(iov,iov_phy)

			if(nalign) {
				if(addr == 0) {
					if(*(int*)iov_phy) {
						// bad iov
						return -1;
					}
				}
				// not eight bytes aligned, adjust iov_phy to unaligned position
				iov_phy = (IOV*)((uintptr_t)iov_phy - 4);
			}
		}
	}

	// first iov
	size -= off;
	if(addr == 0) {
		addr = (uintptr_t)GETIOVBASE(iov_phy);
	}
	addr += off;

	// mapping loop
	nparts_max = *pnparts;
	bytes = 0;
	nparts = 0;

	do {
	
		// boundry check
		last = addr + size - 1;
		if(addr > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)addr, last, bound))) {
			if(size != 0) {
				return -1;
			}
		}

		if(addr < VM_USER_SPACE_BOUNDRY) {
			// system process and kernel address space
			SETIOV(niov, (void *)addr, size);
			bytes += size;
			size = 0;
			niov ++;
			nparts ++;
		} else {
			// make sure l1pidx != newidx the first time through the
			// loop below
			unsigned 	l1pidx = ~0; 
			unsigned	pg_size;
			unsigned	pg_offset;
			pte_t		*pde = 0;
			pte_t		*ptep = 0;
			pte_t		*pteop = 0;

			do {
				unsigned newidx = L1PAGEIDX(addr);

				if(l1pidx != newidx) {
					pde = PDE_ADDR(pgdir[newidx]);
					if(pde == 0) {
						return vmm_map_addr_general(actprp, prp,  piov, pparts, poff, orig_niov, pnparts, flags);\
					}
					l1pidx = newidx;
				}
				pteop = &pde[L2PAGEIDX(addr)];
				pg_size = PTE_PGSIZE(pteop);
				pg_offset = (uintptr_t)addr & (pg_size - 1);
				addr_phy = PTE_PADDR(pteop) + pg_offset;
				if(!PTE_CACHEABLE(pteop)) {
					/* Fix for PR 12533 extended to the uncacheable region mapping to address PR 8392 */
					return vmm_map_addr_general(actprp, prp,  piov, pparts, poff, orig_niov, pnparts, flags);
				}
				if(!PTE_PRESENT(pteop) || (flags & MAPADDR_FLAGS_WRITE) ?
				  !PTE_WRITEABLE(pteop) : !PTE_READABLE(pteop)) {
					/* 
						We use the original niov (orig_niov), since the first page mapping mungs it.
						This addresses PR 12533
					*/
					return vmm_map_addr_general(actprp, prp,  piov, pparts, poff, orig_niov, pnparts, flags);
			 	}

				len = pg_size - pg_offset;
				for( ;; ) {
					uintptr_t	next_addr;
					
					if(len >= size) break;
					next_addr = addr + len;
					if(L1PAGEIDX(next_addr) != l1pidx) break;
					ptep = &pde[L2PAGEIDX(next_addr)];
					// check physical mem continuty
					if((PTE_PERMS(ptep)) != PTE_PERMS(pteop)) break;
					if((PTE_PADDR(pteop) + pg_size) != PTE_PADDR(ptep)) break;
	
					len += pg_size;
					pteop = ptep;
					pg_size = PTE_PGSIZE(ptep);
				}

				/*	AM Note:
					If the phyaddr+size (think crossing 1:1 bound) is outside the
					1:1 area of the kernel, we need to map it in instead
					of just touching it
				*/
				if((addr_phy + min(len,size)) >= VM_KERN_LOW_SIZE) {
					return vmm_map_addr_general(actprp,prp,piov,pparts,poff,orig_niov,pnparts,flags);
				}
	
				// mapped area is bigger?
				if(len >= size) {
					// one source iov is ended
					SETIOV(niov, (uintptr_t)addr_phy, size);
					bytes += size;
					niov ++;
					nparts ++;
					size = 0;
					break;
				}

				SETIOV(niov, (uintptr_t)addr_phy, len);
				addr += len;
				size -= len;
				bytes += len;
				niov ++;
				nparts ++;
			} while(nparts < nparts_max);
		} //end of if kernel space

		if(nparts >= nparts_max) {
			if(size == 0) parts --;
			break;
		}

		// need to check the address of next source iov
		do {
			if(--parts == 0) break;
			iov_phy ++;
			if((ADDR_OFFSET((unsigned)iov_phy + sizeof(_Int32t)) <= sizeof(_Int32t)) &&
				!(flags & MAPADDR_FLAGS_IOVKERNEL)) {
					int nalign = (unsigned)iov_phy & (__PAGESIZE - 1);

					if(nalign) {
						// not eight bytes aligned, get addr first
						addr = (uintptr_t)GETIOVBASE(iov_phy);
					}
					
					// adjust iov_phy
					iov = (IOV *)ADDR_PAGE((uintptr_t)iov + __PAGESIZE);
					TRANSLATE_IOV_ADDR(iov, iov_phy)													

					if(nalign) {
						// not eight bytes aligned, adjust iov_phy to unaligned position
						iov_phy = (IOV*)((uintptr_t)iov_phy - 4);
					} else {
						addr = (uintptr_t)GETIOVBASE(iov_phy);
					}
			} else {
				addr = (uintptr_t)GETIOVBASE(iov_phy);
			}
			size = GETIOVLEN(iov_phy);
		} while (size == 0);
	} while(parts);

	if(parts != 0) {
		*piov = *piov + (*pparts - parts);
		if((nparts != nparts_max) || (size == 0)) {
			*poff = 0;
		} else {
			*poff = GETIOVLEN(iov_phy) - size;
		} 
	} 
	*pparts = parts;
	*pnparts = nparts;

	return bytes;
}

__SRCVERSION("vmm_map_xfer.c $Rev: 153052 $");
