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

#define emm_configure		smm_configure
#define emm_aspace			smm_aspace
#define emm_debuginfo		smm_debuginfo
#define emm_dup				smm_dup
#define emm_fault			smm_fault
#define emm_mapinfo			smm_mapinfo
#define emm_mcreate			smm_mcreate
#define emm_mdestroy		smm_mdestroy
#define emm_mlock			smm_mlock
#define emm_munlock			smm_munlock
#define emm_mprotect		smm_mprotect
#define emm_msync			smm_msync
#define emm_resize			smm_resize
#define emm_swapper			smm_swapper
#define emm_map_xfer		smm_map_xfer
#define emm_vaddrinfo		smm_vaddrinfo
#define emm_validate		smm_validate
#define emm_madvise			smm_madvise
#define emm_memobj_phys		smm_memobj_phys

MEMMGR memmgr_rte = {
 	0,							// pagesize - filled in from startup
	0,							// fault_pulse_code
	0,							// size of ADDRESS

	#undef MMF
	#define MMF(r, f, p, e)		MMF_DEFN(r, f, p, e)
	MM_FUNCS(emm)
};

__SRCVERSION("nano_memphys.c $Rev: 153052 $");
