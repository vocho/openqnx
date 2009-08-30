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

PROCESS		*xfer_prp;
ptrdiff_t	xfer_diff;

int
vmm_map_xfer(PROCESS *actptp, PROCESS *prp, IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags)
{
	IOV			iovlist[32];
	IOV			*iov;
	unsigned	n;
	unsigned	parts;
	unsigned	off;
	unsigned	nparts;
	unsigned	nparts_max;
	unsigned	nbytes;
	uintptr_t	last;
	uintptr_t	base;
	uintptr_t	len;
	IOV			*oiov    = *piov;
	unsigned	oparts   = *pparts;
	uintptr_t	iov_diff = 0;

	/*
	 * Calculate the remapped process virtual address
	 */
	CRASHCHECK( prp == NULL );	
	CRASHCHECK( prp->memory == NULL );
	
	xfer_prp  = prp;
	xfer_diff = MVA_BASE(prp->memory->cpu.asid);

	/*
	 * Check supplied iov array is valid
	 */
	if (!(flags & MAPADDR_FLAGS_IOVKERNEL)) {
		if ((uintptr_t)oiov < VM_USER_SPACE_BOUNDRY) {
			iov_diff = xfer_diff;
		}

		last = (uintptr_t)oiov + oparts * sizeof(IOV) - 1;
		if ((uintptr_t)oiov > last ||
			(!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY((uintptr_t)oiov, last, prp->boundry_addr))) {
			return -1;
		}

		base = ((uintptr_t)oiov) & ~PGMASK;
		last = ((uintptr_t)(oiov + (uintptr_t)oparts)) & ~PGMASK;
		while (base <= last) {
			if (xfer_memprobe((void*)(base + iov_diff)) != 0) {
				return -1;
			}
			base += __PAGESIZE;
		}
	}

	/*
	 * Skip over supplied offset
	 */
	off = *poff;
	while (off >= (len = GETIOVLEN((IOV*)((uintptr_t)oiov + iov_diff)))) {
		off -= len;
		if(--oparts == 0) { 	/* No more parts. */
			*pnparts = 0;
			return 0;
		}
		++oiov;
	}

	iov  = (IOV *)((uintptr_t)oiov + iov_diff);
	base = (uintptr_t)GETIOVBASE(iov);
	len  = (uintptr_t)GETIOVLEN(iov);
	if (off) {
		base += off;
		len  -= off;
		off   = 0;
	}

	/*
	 * Don't adjust non-pidified addresses
	 */
	if (base >= USER_SIZE) {
		xfer_diff = 0;
		xfer_prp  = 0;
	}

	/*
	 * Adjust each iov base by xfer_diff
	 */
	n = min(sizeof iovlist / sizeof *iovlist, oparts);
	parts  = 0;
	nparts = 0;
	nbytes = 0;
	nparts_max = *pnparts;
	while (nparts < nparts_max) {
 		/*
 		 * Make sure address is within range for process
 		 */
		if (len) {
			last = base + len - 1;
			if (base > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY(base, last, prp->boundry_addr))) {
				return -1;
			}

			SETIOV(niov, (IOV *)((uintptr_t)base + xfer_diff), len);
			nbytes += len;
			nparts++;
			niov++;
		}
		if (++parts >= n)
			break;
		iov++;
		base = (uintptr_t)GETIOVBASE(iov);
		len  = (uintptr_t)GETIOVLEN(iov);
	}

	/*
	 * Update the caller's iov list
	 */
	*piov    = oiov + parts;
	*pparts  = oparts - parts;
	*pnparts = nparts;
	*poff    = off;

	return nbytes;
}

__SRCVERSION("vmm_map_xfer.c $Rev: 198521 $");
