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

int
vmm_memobj_phys(OBJECT *obp, paddr_t pa) {
	struct pa_quantum	*pq;
	int					r;

	pq = pa_alloc_fake(pa, obp->mem.mm.size);
	if(pq == NULL) return ENOMEM;
	r = memobj_pmem_add_pq(obp, 0, pq);
	if(r != EOK) {
		pa_free_fake(pq);
	}
	return r;
}

__SRCVERSION("vmm_memobj_phys.c $Rev: 153052 $");
