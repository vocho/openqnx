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

struct args_vaddrinfo {
	off64_t				off;
	paddr_t				paddr;
	PROCESS				*prp;
	uintptr_t			vaddr;
	struct mm_map		*mm;
	int					init;
	int					found;
	unsigned			pg_offset;
	size_t				len;
};

static void 
ext_vaddrinfo(void *data) {
	struct args_vaddrinfo	*args = data;

	KerextLock();
	KerextStatus(NULL, 
			cpu_vmm_vaddrinfo(args->prp, args->vaddr, &args->paddr, &args->len));
}

static int
get_paddr(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct args_vaddrinfo 	*data = d;
	uintptr_t				vaddr;
	struct mm_map			*mm;
	uint64_t				skip;

	data->paddr = pa_quantum_to_paddr(pq) + data->pg_offset;
	if(pq->blk == PAQ_BLK_FAKE) {
		//We only have a single pa_quantum_fake to cover the whole
		//of the run, so we need to adjust the paddr with the
		//offset from the start.
		skip = data->off - off;
		data->paddr += skip;
		data->len = ((pq->run << QUANTUM_BITS) - (uintptr_t)skip) - data->pg_offset;
	} else {
		data->len = QUANTUM_SIZE - data->pg_offset;
		mm = data->mm;
		vaddr = mm->start + (off - mm->offset);
		data->paddr += ADDR_OFFSET(vaddr);
		if(data->init) {
			// Scan through the physically contiguous run of memory.
			// If the page hasn't been initialized yet, do it now. 
			// A driver might be programing some hardware to DMA 
			// into the memory and we don't want to overwrite
			// that data later...
			for( ;; ) {
				if(!(pq->flags & PAQ_FLAG_INITIALIZED)) {
					(void)memory_reference(&data->mm, vaddr, vaddr|(QUANTUM_SIZE-1), MR_NONE, NULL);
				}
				if(pq->run < 0) break;
				if(pq->run == 1) break;
				vaddr += QUANTUM_SIZE;
				if(vaddr > mm->end) break;
				data->len += QUANTUM_SIZE;
				++pq;
			}
		}
	}
	data->found = 1;
	return -1;
}
	
//FUTURE: Do we need to go into the kernel for this? Have to mux with
//FUTURE: page table destruction if not.

unsigned 
vmm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *paddrp, size_t *lenp, unsigned flags) {
	ADDRESS					*adp;
	struct map_set			ms;
	struct mm_map			*mm;
	OBJECT					*obp;
	struct args_vaddrinfo	data;
	int						r;
	unsigned				prot;
	int status;

	if(CPU_1TO1_IS_VADDR(vaddr)) {
		*paddrp = vaddr - CPU_1TO1_VADDR_BIAS;
		if(lenp != NULL) *lenp = ~0;
		return PROT_READ|PROT_WRITE|PROT_EXEC;
	}

	if(KerextAmInKernel() || (flags & VI_KDEBUG)) {
		return cpu_vmm_vaddrinfo(prp, vaddr, paddrp, lenp);
	}

	prot = PROT_NONE;

	if((prp != NULL) && !(flags & VI_PGTBL) && WITHIN_BOUNDRY(vaddr, vaddr, user_boundry_addr)) {
		data.init = flags & VI_INIT;
		data.found = 0;
		adp = prp->memory;
		status = proc_rlock_adp(prp);
		CRASHCHECK(status == -1);
		r = map_isolate(&ms, &adp->map, (uintptr_t)vaddr, 0, MI_NONE);
		if(r == EOK) {
			mm = ms.first;
			if(mm != NULL) {
				if(mm->obj_ref != NULL) {
					data.mm = mm;
					data.pg_offset = ADDR_OFFSET((uintptr_t)vaddr);
					data.off = mm->offset + (ADDR_PAGE((uintptr_t)vaddr) - mm->start);
					obp = mm->obj_ref->obp;
					memobj_lock(obp);
					memobj_pmem_walk(MPW_PHYS|MPW_SYSRAM, obp, data.off, data.off, 
										get_paddr, &data);
					memobj_unlock(obp);
					// Turn on MAP_PHYS to make sure PROT_NONE isn't returned;
					if(data.found) {
						prot = mm->mmap_flags | MAP_PHYS;
					}
				}
				map_coalese(&ms);
			}
		}
		proc_unlock_adp(prp);
	}

	if(prot == PROT_NONE) {
		// If we didn't find anything in the upper level data structures,
		// check with the CPU specific code.
		data.prp = prp;
		data.vaddr = vaddr;
		prot = __Ring0(ext_vaddrinfo, &data);
	}

	*paddrp = data.paddr;
	if(lenp != NULL) *lenp = data.len;
	return prot;
}

__SRCVERSION("vmm_vaddrinfo.c $Rev: 163915 $");
