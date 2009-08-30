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

// These routines only get used by vmm_mmap() and vmm_munmap()
// if we're on a CPU where CPU_SYSTEM_PADDR_MUST == 0


void
sysaddr_map(void *d) {
	struct mm_pte_manipulate *data = d;
	uintptr_t	va;

	KerextLock();
	va = cpu_sysvaddr_find(va_rover, data->end);
	if(va != VA_INVALID) {
		va_rover = va + (size_t)data->end;
		if(pte_map(NULL, va, va_rover-1, data->prot, NULL, data->paddr, 0) != EOK) {
			va = VA_INVALID;
		}
	}
	data->start = va;
}

__SRCVERSION("mm_sysaddr.c $Rev: 153052 $");
