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

#include "pmm.h"

int
pmm_resize(OBJECT *obp, size_t size) {
	void				*ptr;

	ptr = _srealloc(obp->mem.mm.pmem, obp->mem.mm.size, size);
	if(ptr == NULL && size != 0) {
		return ENOMEM;
	}
	mem_free_size += (unsigned)ROUNDUP(obp->mem.mm.size, 4) - ROUNDUP(size, 4);
	obp->mem.mm.pmem = ptr;
	obp->mem.mm.size = size;
	return EOK;
}

__SRCVERSION("pmm_resize.c $Rev: 153052 $");
