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

//NOTE: This overrides a CPU-independent version, since we need to
//NOTE: do some strangeness to handle vmm_dup() processing.

struct kerargs_data_dup {
	void	*src;
	void	*dst;
	size_t	len;
	size_t	off;
};

static void
ker_data_dup(void *data) {
	struct kerargs_data_dup	*kap = data;
	uint8_t					*src;
	uint8_t					*dst;

	/*
	 * The CPU-independent code in data_dup_run() modifies data->va_off
	 * once the memcpy() for this run is completed.
	 *
	 * If we called that function here (within a kernel __Ring0() call)
	 * and we get preempted before exiting the kernel, the call will be
	 * restarted with the modified data->va_off which will perform the
	 * memcpy() with a bogus address.
	 *
	 * Instead, we inline the memcpy() so that a preemption can safely
	 * restart the call using the same parameters.
	 *
	 * To allow for preemption during long copy operations, we break
	 * the copy into page size chunks and keep track of the current
	 * offset we have copied up to. This means we don't have to lock
	 * the kernel, and if we get preempted we only have to re-copy at
	 * most one page of data.
	 */

	src = (uint8_t *)((size_t)kap->src + kap->off);
	dst = (uint8_t *)((size_t)kap->dst + kap->off);
	while (kap->off < kap->len) {
		size_t		len = min(kap->len - kap->off, __PAGESIZE);

		memcpy(dst, src, len);
		kap->off += len;
		src += len;
		dst += len;
	}
	KerextStatus(0, EOK);
}


int
pte_temp_map(ADDRESS *adp, uintptr_t user_va, struct pa_quantum *pq, 
			struct mm_map *mm, size_t len, 
			int (*func)(void *, size_t, void *), void *d) {
	paddr_t					paddr;
	uintptr_t				end;
	int						r;
	size_t					piece;
	struct mm_object_ref	*or;
	OBJECT					*obp;
	uintptr_t				vaddr;
	uintptr_t				tmap_start;
	uintptr_t				tmap_end;
	uintptr_t				use_start;
	uintptr_t				use_end;
	unsigned				min_len;

	or = mm->obj_ref;

	if(or->adp != adp) {
		struct kerargs_data_dup	data;

		/*
		 * Creating a temp mapping the parent's page for vmm_dup() to copy.
		 * This won't work with the ARM virtual cache since this temp copy
		 * will create a cache alias of the parent's page and we may end up
		 * copying stale data from the memory page.
		 *
		 * Instead, we copy directly from the parent address since it is
		 * accessible via its MVA address. We need to enter the kernel to
		 * do this copy so that we enable access to the parent's domain.
		 */

		data.src = (void *)(user_va | MVA_BASE(or->adp->cpu.asid));
		data.dst = (void *)user_va;
		data.len = len;
		data.off = 0;
		return __Ring0(ker_data_dup, &data);
	}

	paddr = pa_quantum_to_paddr(pq);

	obp = or->obp;

	if(CPU_1TO1_IS_PADDR(paddr+len)
		&& !(mm->mmap_flags & PROT_NOCACHE)
		&& (((obp->hdr.type != OBJECT_MEM_SHARED) && (obp->hdr.type != OBJECT_MEM_TYPED))
			|| !(obp->mem.mm.flags & MM_SHMEM_SPECIAL))) {
		vaddr = (uintptr_t)paddr + CPU_1TO1_VADDR_BIAS;
		r = func((void *)vaddr, len, d);
		CPU_1TO1_FLUSH(vaddr, len);
		return r;
	}

	piece = len;
	#define MAX_TEMP_MAPPING (64*1024)
	if(piece > MAX_TEMP_MAPPING) piece = MAX_TEMP_MAPPING;
	min_len = colour_mask_shifted + __PAGESIZE;
	if(piece < min_len) piece = min_len;
	if(piece <= adp->tmap_size && 
		((adp->tmap_base >= (user_va + len)) || 
		 ((adp->tmap_base + adp->tmap_size) <= user_va))) {
		tmap_start = adp->tmap_base;
		piece = adp->tmap_size;
	} else {
		for( ;; ) {
			// We don't have to actually allocate the virtual address since
			// we have the aspace write-locked - nobody else can get in here
			// to fiddle things.
			tmap_start = map_find_va(&adp->map, user_va & ~colour_mask_shifted, 
									piece, colour_mask_shifted, 0);
			if(tmap_start != VA_INVALID) break;
			piece >>= 1;
			if(piece < min_len) {
				// When we created the aspace, we snipped off some pages
				// at the end of the legal user address space range
				// to use as a last chance temp mapping area (see vmm_mcreate.c).
				// We only use them as a last resort because:
				// 1. It's minimum size - we can be more efficient if we can
				//    do the whole temp mapping range in one go.
				// 2. Since the vaddr is off in the nether regions, we'd likely
				//    have to allocate an L2 page table entry to cover it that
				//    we normally wouldn't have. By originally trying vaddrs
				//    in the area of the actual user address first, we're likely
				//    to be able to share an L2 with one we're using anyway.
				piece = min_len;
				tmap_start = adp->map.end + 1;
				break;
			}
			// We might have been passed in a non power-of-two number of pages.
			piece = ROUNDUP(piece, __PAGESIZE);
		}
		adp->tmap_base = tmap_start;
		adp->tmap_size = piece;
	}
	tmap_end = tmap_start + piece - 1;
	use_start = tmap_end;
	use_end   = tmap_start;

	for( ;; ) {
		vaddr = tmap_start;
		piece = (tmap_end - vaddr) + 1;
		if(len < piece) piece = len;
		end = vaddr + piece - 1;
		r = pte_map(adp, vaddr, end, mm->mmap_flags|(PROT_READ|PROT_WRITE), 
				obp, paddr, PTE_OP_TEMP);
		if(r != EOK) break;
		if(vaddr < use_start) use_start = vaddr;
		if(end   > use_end)   use_end   = end;
		r = func((void *)vaddr, piece, d);
		if(r != EOK) break;
		user_va += piece;
		paddr += piece;
		len -= piece;
		if(len == 0) break;
	} 
	if(use_start < use_end) { // in case pte_map() failed on first iteration
		pte_unmap(adp, use_start, use_end, obp);
	}
	return r;
}

__SRCVERSION("mm_temp_map.c $Rev: 172513 $");
