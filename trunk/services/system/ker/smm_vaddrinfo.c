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

#include "externs.h"

unsigned
smm_vaddrinfo(PROCESS *prp, uintptr_t vaddr, paddr_t *paddrp, size_t *lenp, unsigned flags) {
    if(CPU_VADDR_IN_RANGE(vaddr)) {
		*paddrp = CPU_V2P(vaddr);
		if(lenp != NULL) *lenp = (paddr_t)0 - *paddrp;
		return PROT_READ|PROT_WRITE|PROT_EXEC;
	}
	return PROT_NONE;
}

__SRCVERSION("smm_vaddrinfo.c $Rev: 153052 $");
