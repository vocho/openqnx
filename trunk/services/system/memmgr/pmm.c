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


#define pmm_configure		smm_configure
#define pmm_aspace			smm_aspace
#define pmm_debuginfo		smm_debuginfo
#define pmm_dup				smm_dup
#define pmm_fault			smm_fault
#define pmm_map_xfer		smm_map_xfer
#define pmm_mcreate			smm_mcreate
#define pmm_mlock			smm_mlock
#define pmm_munlock			smm_munlock
#define pmm_mprotect		smm_mprotect
#define pmm_vaddrinfo		smm_vaddrinfo
#define pmm_swapper			smm_swapper
#define pmm_validate		smm_validate
#define pmm_madvise			smm_madvise
#define pmm_memobj_phys		smm_memobj_phys

MEMMGR memmgr_phys = {
	sizeof(void *),		// no paging so use size of a pointer for pagesize
	0,					// fault_pulse_code
	0,					// sizeof ADDRESS

	#undef MMF
	#define MMF(r,f,p,e)	MMF_DEFN(r,f,p,e)
	MM_FUNCS(pmm)
};

__SRCVERSION("pmm.c $Rev: 153052 $");
