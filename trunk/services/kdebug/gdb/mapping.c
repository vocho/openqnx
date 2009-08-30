/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include "kdebug.h"

// This routines might be overridden by CPU specific ones.

/*
 * Set up a mapping for reading/writing
 */
void *
mapping_add(uintptr_t vaddr, size_t request_len, unsigned prot, size_t *valid_len) {
	paddr_t		paddr;
	size_t		size;

	if(vaddrinfo(NULL, vaddr, &paddr, &size) == PROT_NONE) return(NULL);

	return cpu_map(paddr, request_len, prot, -1, valid_len);
}


/*
 * Tear down a mapping from above
 */
void
mapping_del(void *p, size_t len) {
	cpu_unmap(p, len);
}
