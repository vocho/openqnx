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


/*
 * Note: this is a generic version of the function; however, for speed,
 * it is overridden by a cpu-specific one. Look in the memmgr/$CPU dir
 * for the src.
 */
OBJECT *
vmm_vaddr_to_memobj(PROCESS *prp, void *vaddr, unsigned *offset, int create) {
	paddr_t						paddr;
	struct pa_quantum			*pq;
	unsigned					prot;

	//RUSH3: This won't work once we start swapping out. Problem
	//RUSH3: With using object & offset though - offset is 64 bits and we
	//RUSH3: only have room for 32. Also, this is be called from the
	//RUSH3: kernel. Will we have race conditions walking the structures?
	prot = cpu_vmm_vaddrinfo(prp, (uintptr_t)vaddr, &paddr, NULL);
#ifndef NDEBUG
	if(prot == PROT_NONE) crash();
#endif	

	if(create) {
		//RUSH3: Turn on a bit in the object saying that it has sync objects
		//RUSH3: in it? Would allow us to handle MAP_LAZY/paged out stuff better.	

		// whack in our sync flag, so on free we can avoid MemobjDestroy stuff 
		pq = pa_paddr_to_quantum(paddr);
		if(pq != NULL) {
#ifndef NDEBUG		
			if(!(pq->flags & PAQ_FLAG_INITIALIZED) 
				&& WITHIN_BOUNDRY((uintptr_t)vaddr, (uintptr_t)vaddr, user_boundry_addr)) {
				crash();
			}
#endif		
			pq->flags |= PAQ_FLAG_HAS_SYNC;
		}
	}

	*offset = PADDR_TO_SYNC_OFF(paddr);
	return PADDR_TO_SYNC_OBJ(paddr);
}

__SRCVERSION("vmm_vaddr_to_memobj.c $Rev: 153052 $");
