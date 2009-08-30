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

#include <sys/mman.h>
#include <arm/mmu.h>
#include "kdintl.h"


void
cpu_init_map(void) {
}


void *
cpu_map(paddr64_t paddr, unsigned size, unsigned prot, int colour, unsigned *mapped_len) {
	return NULL;
}


void
cpu_unmap(void *p, unsigned size) {
}


unsigned
cpu_vaddrinfo(void *p, paddr64_t *paddrp, unsigned *lenp) {
	paddr_t		va  = (uintptr_t)p;
	unsigned	off = va & PGMASK;
	pte_t		pte;

	if (*VTOPDIR(va) == 0 || ((pte = *VTOPTEP(va)) & ARM_PTE_VALID) == 0) {
		return PROT_NONE;
	}
	*paddrp = pte & PGMASK;
	*lenp = __PAGESIZE - off;
	//ZZZ examine PTE and return proper protection bits
	return PROT_READ|PROT_WRITE|PROT_EXEC;
}
