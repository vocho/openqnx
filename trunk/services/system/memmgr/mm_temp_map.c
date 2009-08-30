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

//NOTE: This function may be overriden with CPU specific code
//NOTE: on some systems (e.g. ARM).

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

	paddr = pa_quantum_to_paddr(pq);
	vaddr = (uintptr_t)paddr + CPU_1TO1_VADDR_BIAS;

	or = mm->obj_ref;
	obp = or->obp;

	if(CPU_1TO1_IS_PADDR(paddr+len) 
#if CPU_SYSTEM_HAVE_COLOURS		
		&& ((vaddr & colour_mask_shifted) == (user_va & colour_mask_shifted))
#endif		
		&& !(mm->mmap_flags & PROT_NOCACHE)
		&& (((obp->hdr.type != OBJECT_MEM_SHARED) && (obp->hdr.type != OBJECT_MEM_TYPED)) 
			|| !(obp->mem.mm.flags & MM_SHMEM_SPECIAL))) {
		return func((void *)vaddr, len, d);
	}

	piece = len;
	#define MAX_TEMP_MAPPING (64*1024)
	if(piece > MAX_TEMP_MAPPING) piece = MAX_TEMP_MAPPING;
	min_len = colour_mask_shifted + __PAGESIZE;
	if(piece < min_len) piece = min_len;
	/*
	 * Check that we can use the tmp area and does not overlap
	 */
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
#if CPU_SYSTEM_HAVE_COLOURS		
		vaddr += ((user_va & colour_mask_shifted) - vaddr) & colour_mask_shifted;
#endif		
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
