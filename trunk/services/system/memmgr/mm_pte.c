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

#define MSTATE_SPLIT_START	0
#define MSTATE_SPLIT_END	1
#define MSTATE_MANIPULATE	2
#define MSTATE_MERGE		3

struct pte_manip {
	struct mm_pte_manipulate	data;
	unsigned					state;
	uintptr_t					mapped;
};


static int
do_manipulation(struct pte_manip *manip) {
	int		r;

	if(CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES != VPS_NONE) {
		if(mm_flags & MM_FLAG_VPS) {
			r = EOK;
			switch(manip->state) {
			case MSTATE_SPLIT_START:	
				if(!(manip->data.op & PTE_OP_PREALLOC)) {
					r = cpu_pte_split(manip->data.start, &manip->data);
					if(r != EOK) return r;
					manip->data.start = manip->data.first;
				}
				manip->state = MSTATE_SPLIT_END;
				// fall through
			case MSTATE_SPLIT_END:
				if(!(manip->data.op & PTE_OP_PREALLOC)) {
					r = cpu_pte_split(ADDR_PAGE(manip->data.end+1), &manip->data);
					if(r != EOK) return r;
					manip->data.start = manip->data.first;
				}	
				manip->state = MSTATE_MANIPULATE;
				// fall through
			case MSTATE_MANIPULATE:	
				r = cpu_pte_manipulate(&manip->data);
				manip->mapped = manip->data.start;
				if(r != EOK) return r;
				manip->data.start = manip->data.first;
				manip->state = MSTATE_MERGE;
				// fall through
			case MSTATE_MERGE:	
				if(manip->data.op & (PTE_OP_MAP|PTE_OP_PROT|PTE_OP_FORCEMERGE)) {
					r = cpu_pte_merge(&manip->data);
				} else if(manip->data.split_end > manip->data.end) {
					manip->data.start = manip->data.end + 1;
					r = cpu_pte_merge(&manip->data);
					manip->data.start = manip->data.first;
				}
				break;
			default:
				crash();
				break;
			}
			return r;
		}		
	}

	r = cpu_pte_manipulate(&manip->data);
	manip->mapped = manip->data.start;

	return r;
}


static void 
ker_manipulate(void *data) {
	KerextLock();
	KerextStatus(0, do_manipulation(data));
}


static int 
pte_manipulate(struct pte_manip *manip) {
	int		r;

	if(ADDR_OFFSET(manip->data.start) != 0) crash();
	if(ADDR_OFFSET(manip->data.end+1) != 0) crash();
	if((manip->data.op & PTE_OP_MAP) && (ADDR_OFFSET(manip->data.paddr) != 0)) crash();
			
	manip->state = MSTATE_SPLIT_START;
	manip->mapped = manip->data.first = manip->data.start;
	manip->data.split_end = manip->data.end;

	if(KerextAmInKernel()) {
		return do_manipulation(manip);
	}

	//FUTURE: Do we actually have to go into the kernel all of the time
	//FUTURE: for PTE manipulation? We need to for the system space since
	//FUTURE: the X86 has to walk the mem_dir_list, but maybe not for
	//FUTURE: user space. The address space is usually locked (aside
	//FUTURE: from vmm_resize()) when manipulating the pgtbl, so we
	//FUTURE: might not even need a mutex.
	//FUTURE: Have to worry about SMP, where one CPU might be in a locked
	//FUTURE: kernel while we remove perms to access the memory being
	//FUTURE: referenced.

	manip->data.op |= PTE_OP_PREEMPT;
	do {
		r = __Ring0(ker_manipulate, manip);
	} while(r == EINTR);

	return r;
}


int
pte_map(ADDRESS *adp, uintptr_t start, uintptr_t end, int flags,
			OBJECT *obp, paddr_t paddr, unsigned extra) {
	struct pte_manip			manip;
	int							r;

	manip.data.adp = adp;
	manip.data.start = start;
	manip.data.end = end;
	manip.data.prot = flags;
	manip.data.paddr = paddr;
	manip.data.op = PTE_OP_MAP|extra;
	if((obp != NULL) && ((obp->hdr.type==OBJECT_MEM_SHARED)||(obp->hdr.type==OBJECT_MEM_TYPED))) {
		//Next line is for old libc's that didn't turn on SHMCTL_HAS_SPECIAL
		if(obp->shmem.special != 0) obp->mem.mm.flags |= SHMCTL_HAS_SPECIAL;
		manip.data.shmem_flags = obp->mem.mm.flags;
		manip.data.special = obp->shmem.special;
	} else {
		manip.data.shmem_flags = 0;
	}
	r = pte_manipulate(&manip);
	if((r != EOK) && (manip.mapped > start)) {
		//pte_manipulate failed => need to unmap whatever's been mapped so far.
		manip.data.end   = manip.data.start - 1;
		manip.data.start = start;
		manip.data.op    = PTE_OP_UNMAP;
		(void) pte_manipulate(&manip);
	}
	return r;
}


//RUSH3: We're startlingly close to not needing pte_prot() anymore - just
//RUSH3: two uses: vmm_dup.c & vmm_msync.c. Think about getting rid of it.
int
pte_prot(ADDRESS *adp, uintptr_t start, uintptr_t end, int flags, OBJECT *obp) {
	struct pte_manip	manip;

	manip.data.adp = adp;
	manip.data.start = start;
	manip.data.end = end;
	manip.data.prot = flags;
	manip.data.op = PTE_OP_PROT;
	switch(obp->hdr.type) {
	case OBJECT_MEM_SHARED:	
	case OBJECT_MEM_TYPED:	
		manip.data.shmem_flags = obp->mem.mm.flags;
		manip.data.special = obp->shmem.special;
		break;
	default:	
		manip.data.shmem_flags = 0;
		break;
	}
	return pte_manipulate(&manip);
}


int
pte_unmap(ADDRESS *adp, uintptr_t start, uintptr_t end, OBJECT *obp) {
	struct pte_manip	manip;

	manip.data.adp = adp;
	manip.data.start = start;
	manip.data.end = end;
	manip.data.op = PTE_OP_UNMAP;
	if((obp != NULL) && ((obp->hdr.type==OBJECT_MEM_SHARED)||(obp->hdr.type==OBJECT_MEM_TYPED))) {
		//Next line is for old libc's that didn't turn on SHMCTL_HAS_SPECIAL
		if(obp->shmem.special != 0) obp->mem.mm.flags |= SHMCTL_HAS_SPECIAL;
		manip.data.shmem_flags = obp->mem.mm.flags;
		manip.data.special = obp->shmem.special;
	} else {
		manip.data.shmem_flags = 0;
	}
	return pte_manipulate(&manip);
}


int
pte_prealloc(ADDRESS *adp, uintptr_t start, uintptr_t end) {
	struct pte_manip	manip;

	manip.data.adp = adp;
	manip.data.start = start;
	manip.data.end = end;
	manip.data.op = PTE_OP_PREALLOC;
	manip.data.shmem_flags = 0;
	return pte_manipulate(&manip);
}


int
pte_bad(ADDRESS *adp, uintptr_t vaddr) {
	struct pte_manip	manip;

	manip.data.adp = adp;
	vaddr = ADDR_PAGE(vaddr);
	manip.data.start = vaddr;
	manip.data.end = vaddr | (__PAGESIZE-1);
	manip.data.op = PTE_OP_BAD;
	manip.data.prot = 0;
	manip.data.shmem_flags = 0;
	manip.data.paddr = 0;
	return pte_manipulate(&manip);
}

__SRCVERSION("mm_pte.c $Rev: 172513 $");
