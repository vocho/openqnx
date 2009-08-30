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


//RUSH3: This variables are referenced by the PPC600 kernel source, but not
//RUSH3: by the the other families. The non-600 message passing code could
//RUSH3: be cleaned up by removing the references to some of them. 
//RUSH3: 'xfer_prp' needs to be made per-CPU if we allow multiple msg passes
//RUSH3: at the same time on SMP.
PROCESS		*xfer_prp;
uintptr_t	xfer_diff[NUM_XFER_MAPPINGS];
uintptr_t	xfer_lastaddr[NUM_XFER_MAPPINGS];
unsigned	xfer_rotor;

unsigned			zp_flags = ZP_CACHE_OFF;
static unsigned		dcache_lsize;

void
zero_page(void * s, size_t len, struct mm_map *mm) {
	unsigned 				flags;
	struct mm_object_ref	*or;
	OBJECT					*obp;

	flags = zp_flags;
	if(mm != NULL) {
		flags |= mm->mmap_flags & ZP_NOCACHE;
		//Need to check if the object is special and if the I bit
		//is being turned on. If so, turn on ZP_NOCACHE.
		or = mm->obj_ref;
		if(or != NULL) {
			obp = or->obp;
			if((obp->hdr.type == OBJECT_MEM_SHARED) && (obp->shmem.special & PPC_SPECIAL_I)) {
				flags |= ZP_NOCACHE;
			}
		}
	}
	if((flags & (ZP_CACHE_OFF|ZP_DCBZ_BAD|ZP_NOCACHE)) == 0) {
		__asm__ __volatile__(
			"1:			;"
			"	dcbz 	0,%1;"
			"   sub.	%3,%3,%2;"
			"	add 	%1,%1,%2;"
			"	bne+	1b;"
			: "=&b" (s)
			: "0" (s), "r"(dcache_lsize), "r"(len)
			);
	} else {
		if((flags & (ZP_CACHE_OFF|ZP_CACHE_PURGE|ZP_NOCACHE)) == (ZP_CACHE_PURGE|ZP_NOCACHE)) {
			CacheControl(s, len, MS_INVALIDATE);
		}
		__asm__ __volatile__(
			"1:			;"
			"	stw	%3,0(%1);"
			"	stw	%3,4(%1);"
			"	stw	%3,8(%1);"
			"	stw	%3,12(%1);"
			"	stw	%3,16(%1);"
			"	stw	%3,20(%1);"
			"	stw	%3,24(%1);"
			"	stw	%3,28(%1);"
			"	addi %1,%1,32;"
			"	bdnz+	1b;"
			: "=&b" (s)
			: "0" (s), "c"(len>>5), "r"(0)
			);
	}
}

void
vmm_init_mem(int phase) {
	switch(phase) {
	case 0:
		dcache_lsize = SYSPAGE_ENTRY(cacheattr)[SYSPAGE_ENTRY(cpuinfo)->data_cache].line_size;

		pa_init(1);

		fam_pte_init(phase);

		copy_vm_code();
		break;

	case 1:
		pa_start_background();
		fault_init();
		lock_init();
		break;

	case 2:
		// Init second, third, etc CPU's memmgr regsisters
		fam_pte_init(phase);
		break;

	default:
		break;
	}
}

__SRCVERSION("vmm_init_mem.c $Rev: 153052 $");
