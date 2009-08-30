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

//#define CHECK_SLOT_TLBS

struct xfer_cache {
	ADDRESS			*act_adp;
	ADDRESS			*other_adp;
	uintptr_t		lo;
	uintptr_t		hi;
	ptrdiff_t		diff;
};

static unsigned				tlb_xfer_rotor[PROCESSORS_MAX];
static unsigned				need_xfer_clean[PROCESSORS_MAX];
static struct xfer_map		*xfer_curr;
static struct xfer_map		*xfer_end;
static unsigned				xfer_clean_per_slot;
static struct xfer_cache	last_xfer;

#define XFER_RANGE_END		(MIPS_R4K_K3BASE+MIPS_R4K_K3SIZE)

uintptr_t
xfer_init(uintptr_t free_base, uintptr_t max_pgsz) {
	unsigned		num_slots;
	struct xfer_map	*xfer;
	unsigned		xfer_size;
	uintptr_t		max_pgsz2 = max_pgsz * 2;

	CRASHCHECK(max_pgsz2 >= MIPS_R4K_K3SIZE);
	if(max_pgsz2 < KERN_XFER_SLOT_SIZE) max_pgsz2 = KERN_XFER_SLOT_SIZE;
	free_base = ROUNDUP(free_base, max_pgsz2);
	xfer_size = num_tlbs * KERN_XFER_SLOT_SIZE;
	if(xfer_size < max_pgsz2) xfer_size = max_pgsz2;
	if((XFER_RANGE_END - free_base) < xfer_size) {
		xfer_size = XFER_RANGE_END - free_base;
		max_pgsz2 = xfer_size;
	}
	num_slots = xfer_size / KERN_XFER_SLOT_SIZE;
	xfer = _smalloc(sizeof(*xfer_tbl) * num_slots);
	if(xfer == NULL) crash();
	xfer_tbl = xfer_curr = xfer;
	xfer_end = &xfer[num_slots];
	kern_xfer_base = free_base;
	do { 
		xfer->xferbase = free_base;
		xfer->slots = 1;
		free_base += KERN_XFER_SLOT_SIZE;
		++xfer;
	} while(xfer != xfer_end);
	kern_xfer_end = free_base - 1;
	xfer_clean_per_slot = (num_tlbs + (num_slots - 1)) / num_slots;
	return max_pgsz2 / 2;
}


void
xfer_cache_kill(ADDRESS *adp) {
	if(adp == last_xfer.act_adp) last_xfer.act_adp = NULL;
	if(adp == last_xfer.other_adp) last_xfer.other_adp = NULL;
}


static void
update_slots(void) {
	unsigned		i;
	unsigned		clean;

	clean = xfer_clean_per_slot;
	InterruptDisable();
	if(++xfer_curr == xfer_end) {
		xfer_curr = xfer_tbl;
	}
	clean *= xfer_curr->slots;
	xfer_curr->slots = 1;

	for(i = 0; i < NUM_PROCESSORS; ++i) {
		need_xfer_clean[i] += clean;
	}
	InterruptEnable();
}


static void
clean_slots(void) {
	struct r4k_tlb  tlb;
	uintptr_t       va;
	int				count;
	unsigned		my_cpu = RUNCPU;
	uintptr_t		start = kern_xfer_base;
	uintptr_t		end = kern_xfer_end;
	unsigned		idx;

	count = need_xfer_clean[my_cpu];
	if(count > num_tlbs) count = num_tlbs;
	idx = tlb_xfer_rotor[my_cpu];
	while(count > 0) {
		/* invalidate one tlb slot */
		r4k_gettlb(&tlb, idx << TLB_IDX_SHIFT);
		va = tlb.hi & ~TLB_HI_ASIDMASK;
		if((va >= start) && (va <= end)) {
			// Kill the entry by pointing into a set kseg0 location.
			r4k_settlb(MIPS_R4K_K0BASE + (idx * (__PAGESIZE * 2)), 
					0, 0, pgmask_4k, idx << TLB_IDX_SHIFT);
		}
		if(++idx >= num_tlbs) {
			/* wrap around */
			idx = 0;
		}
		--count;
	}
	tlb_xfer_rotor[my_cpu] = idx;
	need_xfer_clean[my_cpu] = 0;
}


/*
 * map_range: map addr in prp's address space into the kernel
 *                   by putting a pointer to prp's appopriate page table
 *                   in the upper half of actprp's level 1 page table.
 *                   Choose the next available slot(s) in the xfer_tbl
 *                   to get the kernel address for the xfer.
 */
static ptrdiff_t
map_range(PROCESS *actprp, PROCESS *prp, uintptr_t addr, uintptr_t *lo, uintptr_t *hi) {
	uintptr_t           otheraddr;
	pte_t               **root1, **root2;
	pte_t				*pt;
	ADDRESS             *adp;
	unsigned			map_size;
	ptrdiff_t			diff;
	unsigned			num_slots;
	struct xfer_map		*xfer_start;

	CRASHCHECK(actprp == prp);

	if(MIPS_IS_KSEG0(addr)) {
		/* no mapping necessary */
		*lo = MIPS_R4K_K0BASE;
		*hi = MIPS_R4K_K1BASE;
		return 0;
	}

	adp = prp->memory;
	root1 = &adp->cpu.pgdir[L1IDX(addr)];
	pt = *root1;
	if(pt == NULL) {
		map_size = KERN_XFER_SLOT_SIZE;
	} else {
		map_size = PGMASK_TO_PGSIZE(pt->pm)*2;
		if(map_size < KERN_XFER_SLOT_SIZE) map_size = KERN_XFER_SLOT_SIZE;
	}

	otheraddr = ROUNDDOWN(addr, map_size);

	if((adp == last_xfer.other_adp) 
		&& (actprp->memory == last_xfer.act_adp)
		&& (addr >= last_xfer.lo)
		&& (addr < last_xfer.hi)) {

		// Same address spaces as last time and we're still in range
		// so we can re-use the last translation.
		*lo = last_xfer.lo;
		*hi = last_xfer.hi;
		return last_xfer.diff;
	}


	// clear old mapping cache to prevent preemption problems
	last_xfer.act_adp = NULL;

	// Find a slot with the proper alignment for the page size
	do {
		update_slots();
	} while((xfer_curr->xferbase & (map_size-1)) != 0);

	root1 = &adp->cpu.pgdir[L1IDX(otheraddr)];

	/* 
	 * record for caller the lowest and highest accessible 
	 * addresses in the inactive address space. 
	 */
	*lo = otheraddr;
	*hi = otheraddr + map_size;

	xfer_start = xfer_curr;
	num_slots = 1;
	diff = xfer_curr->xferbase - otheraddr;
	root2 = &actprp->memory->cpu.pgdir[L1IDX(xfer_curr->xferbase)];
	for( ;; ) {
		/*
		 * Transfer the L2 page table for the inactive aspace to the
		 * active page table.
		 * Also store the difference between the actual address and the
		 * address used to do the copy, as well as the other 
		 * prp, since if/when a page fault happens during the xfer,
		 * we only know the active process.
		 */
		*root2 = *root1;
		xfer_curr->diff = diff;
		xfer_curr->prp = prp;
		map_size -= KERN_XFER_SLOT_SIZE;
		if(map_size == 0) break;
		update_slots();
		CRASHCHECK(xfer_curr == xfer_tbl); // We shouldn't wrap around
		++num_slots;
		++root1;
		++root2;
	}
	xfer_start->slots = num_slots;

	clean_slots();

#ifdef CHECK_SLOT_TLBS
	{
		unsigned			idx;
		struct r4k_tlb		tlb;
		volatile uintptr_t  va;
		volatile uintptr_t	va_end;
		volatile unsigned	size;
		volatile uintptr_t	start;
		volatile uintptr_t	end;

		// Make sure that we don't have any stale entries in the TLB
		// for the xfer range we're about to use.

		start = otheraddr + diff;
		end = xfer_curr->xferbase + (KERN_XFER_SLOT_SIZE - 1);
		for(idx = 0; idx < num_tlbs; ++idx) {
			r4k_gettlb(&tlb, idx << TLB_IDX_SHIFT);
			size = PGMASK_TO_PGSIZE(tlb.pmask)*2;
			va = tlb.hi & ~(size - 1);
			va_end = va + size - 1;
			if(!((va_end < start) || (end < va))) {
				crash();
			}
		}
	}
#endif	

	last_xfer.lo = *lo;
	last_xfer.hi = *hi;
	last_xfer.diff = diff;
	last_xfer.other_adp = adp;
	// Make sure the 'act_adp' field is set _last_ so that the structure
	// contents are always consistent when we 'activate' the cache info.
	last_xfer.act_adp = actprp->memory;

	return diff;
}


int 
vmm_map_xfer(PROCESS *actprp, PROCESS *prp,  IOV **piov, int *pparts, 
					int *poff, IOV *niov, int *pnparts, unsigned flags) {
	uintptr_t 	lo;
	uintptr_t 	hi;
	IOV 		*iov;
	IOV 		*iov_o;
	IOV 		*iov_e;
	IOV 		*iov_a;
	unsigned	off, nparts, nparts_max;
	unsigned	num_inparts;
	unsigned	nbytes;
	uintptr_t	last, base, len;
	ptrdiff_t	diff;
	unsigned	remainder;
	IOV 		iovlist[32];

	if(!(flags & MAPADDR_FLAGS_NOTFIRST)) {
		// If this is the first time we're calling this function
		// for a particular message pass, clear the last_xfer cache,
		// since a user process may have done mmap/munmap's that have
		// invalidated the information.
		last_xfer.act_adp = NULL;
	}

	// map in iov
	iov_o = *piov;
	iov_e = &iov_o[*pparts];

	off = *poff;
	lo = 0;
	hi = 0;
	diff = 0;
	// By checking the MAP_ADDR_FLAGS_SYSPRP flag here, we're making
	// the (currently valid) assumption that all iov's from procnto are
	// in the system address space.
	if(flags & (MAPADDR_FLAGS_IOVKERNEL|MAPADDR_FLAGS_SYSPRP)) {

		// skip over offsets
		for( ;; ) {
			len = GETIOVLEN(iov_o);
			if(off < len) break;
			off -= len;
			++iov_o;
			if(iov_o >= iov_e) {    /* No more parts. */
				*pnparts = 0;
				return 0;
			}
		}
		iov = iov_o;
		num_inparts = iov_e - iov_o;
	} else {
		// The IOV list is in the non-active user address space, so we
		// have to establish mappings in the active address space to
		// check the contents.
#ifndef NDEBUG
		// boundry check
		last = (uintptr_t)iov_e - 1;
		if(((uintptr_t)iov_o > last) || !WITHIN_BOUNDRY((uintptr_t)iov_o, last, prp->boundry_addr)) {
			return -1;
		}
#endif
		iov_a = NULL;
		for( ;; ) {
			unsigned	left;
			uintptr_t	mapping_point;

			mapping_point = (uintptr_t)iov_o;
			remainder = 0;
			for( ;; ) {
				if((mapping_point < lo) || (mapping_point >= hi)) {

					diff = map_range(actprp, prp, mapping_point, &lo, &hi);
					iov_a = (void *)((uintptr_t)iov_o + diff);
					last = (uintptr_t)iov_a + ((uintptr_t)iov_e - (uintptr_t)iov_o);
					if((hi+diff) < last) last = (hi+diff);

					// check iov readability
					base = mapping_point+diff;
					for( ;; ) {
						if(base > last) break;
						if(xfer_memprobe((void *)base) != 0) {
							return -1;
						}
						base += __PAGESIZE;
					}
				}

				left = hi - mapping_point - 1;
				if(left >= sizeof(*iov_o)) break;
				// We don't have enough space in the mapping range to bring
				// in the whole IOV entry. Copy the first part into the
				// temporary iovlist and loop around to establish a new
				// range to get the rest.
				memcpy(iovlist, iov_a, left);
				remainder = left;
				mapping_point += sizeof(*iov_o) - remainder;
			}

			// skip over offsets
			if(remainder != 0) {
				// The IOV was split across a mapping range.
				// We saved the first part in 'iovlist' above, now copy
				// the rest of the entry into it so we can get the length.
				memcpy((uint8_t *)iovlist + remainder, 
						(uint8_t *)iov_a + remainder, sizeof(*iov_a));
				len = GETIOVLEN(iovlist);
			} else {
				len = GETIOVLEN(iov_a);
			}
			if(off < len) break;
			off -= len;
			++iov_a;
			++iov_o;
			if(iov_o >= iov_e) {    /* No more parts. */
				*pnparts = 0;
				return 0;
			}
		}
		num_inparts = (IOV *)last - iov_a;
		if(num_inparts > NUM_ELTS(iovlist)) {
			num_inparts = NUM_ELTS(iovlist);
			last = (uintptr_t)&iov_a[NUM_ELTS(iovlist)];
		}
		memcpy((uint8_t *)iovlist + remainder, 
				(uint8_t *)iov_a + remainder, last - (uintptr_t)iov_a - remainder);
		iov = iovlist;
	}

	base = (uintptr_t)GETIOVBASE(iov);
	len = GETIOVLEN(iov);
	if(off) {
		base += off;
		len -= off;
		off = 0;
	}

	nparts = 0;
	nbytes = 0;
	nparts_max = *pnparts;
	for( ;; ) {
		if(nparts >= nparts_max) break;
		/* Make sure address is within range for process */
		if(len) {
			last = base + len - 1; 
			if(base > last || (!(flags & MAPADDR_FLAGS_SYSPRP) && !WITHIN_BOUNDRY(base, last, prp->boundry_addr))) {
				if(len != 0) {
					return -1;
				}
			}

			if((base < lo) || (base >= hi)) {
				if(nparts != 0) break;
				diff = map_range(actprp, prp, base, &lo, &hi);
			}

			if(last >= hi) {
				// The iov is longer than will fit than one mapping.
				len -= (last - hi) + 1;
				SETIOV(niov, (IOV *)(base + diff), len);
				++nparts;
				off = len + (base - (uintptr_t)GETIOVBASE(iov));
				nbytes += len;
				break;
			}

			SETIOV(niov, (IOV *)(base + diff), len);
			nbytes += len;
			++nparts;
			++niov;
		}
		++iov_o;
		if(--num_inparts == 0) break;
		++iov;
		base = (uintptr_t)GETIOVBASE(iov);
		len = GETIOVLEN(iov);
	}

	*piov = iov_o;
	*pparts = iov_e - iov_o;
	*pnparts = nparts;
	*poff = off;

	return nbytes;
}

__SRCVERSION("vmm_map_xfer.c $Rev: 199381 $");
