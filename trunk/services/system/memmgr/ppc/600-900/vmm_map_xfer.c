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

pte_t	**xfer_pgdir;


#define SR_MAP_MASK		0xf0000000
#define MSG_SR_BASE0	0x10000000
#define MSG_SR_BASE1	0x20000000

/*
 * MAP_XFER function
 * This function is critical to system performance. It is used for most
 * message passing calls between processes. It should be as optimized as
 * possible...
 *
 * We map message passing addresses into two temporary 256MB regions:
 * One at 0x10000000, and the other at 0x20000000
 * The corresponding user addresses are base0 and base1
 * Each of these regions represents an alias of a 256MB region in the target
 * process address space. Due to the MMU architecture of the 60x/7xx processors,
 * the MMU lookups and hits will be identical to the what is stored for the
 * target process. This makes this scheme particularly efficient.
 */

int
vmm_map_xfer(PROCESS *actprp, PROCESS *prp,  IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags) {
	unsigned	base0, base1;
	IOV 		*iov_phy, *iov;
	void 		*addr;
	unsigned	parts, size, addr_phy, nparts, nparts_max, bytes, bound;
	pte_t		**pgdir;
	uintptr_t	last, off;

#ifndef NDEBUG
	ADDRESS							*adp;

	if(!prp || !(adp = prp->memory) || !(pgdir = adp->cpu.pgdir)) {
		return -1;
	}
#else
	pgdir = prp->memory->cpu.pgdir;
#endif
	xfer_pgdir = pgdir;
	xfer_diff[0] = xfer_diff[1] = 0;
	base0 = base1 = 0;
	xfer_prp = prp;

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
			// User address
			base0 = (unsigned)iov & SR_MAP_MASK;
			iov_phy = (IOV *)(((uintptr_t)iov & ~SR_MAP_MASK) + MSG_SR_BASE0);
			xfer_diff[0] = MSG_SR_BASE0 - base0;
			// Set Segment register
			MSG_XFER_SET_SR(1, (ADDRESS *)prp->memory, base0);
		}
	} else {
		iov_phy = iov;
	}

	// skip offsets
	off = *poff;
	while( off >= (size = GETIOVLEN(iov_phy)) ) {
		off -= size;
		if(--parts == 0) { 	/* No more parts. */
			*pnparts = parts;
			return 0;
		}
		++iov_phy;
	}

	// first iov
	size -= off;
	addr = (char *)GETIOVBASE(iov_phy) + off;

	// mapping loop
	nparts_max = *pnparts;
	bytes = 0;
	nparts = 0;

	do {
		int len;
		// boundry check
		last = (uintptr_t)addr + size - 1;
		if((uintptr_t)addr > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)addr, last, bound))) {
			return -1;
		}

		if(((uintptr_t)addr & SR_MAP_MASK) == (((uintptr_t)addr+size-1) & SR_MAP_MASK)) {
			len = size;
		} else {	
			len = (((uintptr_t)addr & SR_MAP_MASK) + 0x10000000) - (uintptr_t)addr;
		}
				
		if((uintptr_t)addr < VM_USER_SPACE_BOUNDRY) {
			// system process and kernel address space
			SETIOV(niov, addr, size);
			bytes += size;
			size = 0;
			niov ++;
			nparts ++;
		} else if (((uintptr_t)addr & SR_MAP_MASK) == base0) {
			
			addr_phy = (unsigned)(((uintptr_t)addr & ~SR_MAP_MASK) + MSG_SR_BASE0);
			SETIOV(niov, addr_phy, len);
			bytes += len;
			size -= len;
			niov ++;
			nparts ++;
		} else if (((uintptr_t)addr & SR_MAP_MASK) == base1) {
			
			addr_phy = (unsigned)(((uintptr_t)addr & ~SR_MAP_MASK) + MSG_SR_BASE1);
			SETIOV(niov, addr_phy, len);
			bytes += len;
			size -= len;
			niov ++;
			nparts ++;
		} else {
			if(!base0) {
				base0 = (unsigned)addr & SR_MAP_MASK;
				xfer_diff[0] = MSG_SR_BASE0 - base0;
				// Set Segment register
				MSG_XFER_SET_SR(1, (ADDRESS *)prp->memory, base0);
				addr_phy = (unsigned)(((uintptr_t)addr & ~SR_MAP_MASK) + MSG_SR_BASE0);
				SETIOV(niov, addr_phy, len);
				bytes += len;
				size -= len;
				niov ++;
				nparts ++;
			} else if(!base1) {
				base1 = (unsigned)addr & SR_MAP_MASK;
				xfer_diff[1] = MSG_SR_BASE1 - base1;
				// Set Segment register
				MSG_XFER_SET_SR(2, (ADDRESS *)prp->memory, base1);
				addr_phy = (unsigned)(((uintptr_t)addr & ~SR_MAP_MASK) + MSG_SR_BASE1);
				SETIOV(niov, addr_phy, len);
				bytes += len;
				size -= len;
				niov ++;
				nparts ++;
			} else {
				break;
			}
		} //end of if kernel space

		if(nparts >= nparts_max) {
			if(size == 0) parts --;
			break;
		}

		if(size != 0) {
			// Exception case, we crossed over a 256MB boundary
			addr = (void *)((uintptr_t)addr + len);
			// Size has already been adjusted
			continue;
		}
		// need to check the address of next source iov
		do {
			if(--parts == 0) break;
			iov_phy ++;
			addr = GETIOVBASE(iov_phy);
			size = GETIOVLEN(iov_phy);
		} while (size == 0);
	} while(parts);

	if(parts != 0) {
		*piov = *piov + (*pparts - parts);
		if(size == 0) {
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
