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

void *
cpu_early_paddr_to_vaddr(paddr_t p, unsigned size, paddr_t *l2mem) {
	return (void *)CPU_P2V((uintptr_t)p);
}

unsigned
cpu_whitewash(struct pa_quantum *pq) {
	cpu_colour_clean(pq, COLOUR_CLEAN_ZERO_FLUSH);
	PAQ_SET_COLOUR(pq, PAQ_COLOUR_NONE);
	return PAQ_FLAG_ZEROED;
}

__SRCVERSION("cpu_pa.c $Rev: 153052 $");
