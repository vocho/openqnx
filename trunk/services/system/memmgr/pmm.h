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

#include "mm_internal.h"

//
// Physical memory system structures
//
#undef MMF
#define MMF(r, f, p, e)		MMF_PROTO(r, f, p, e)
MM_FUNCS(pmm)

struct mem_phys_entry {
	struct mem_phys_entry			*next;
	size_t							size;
	unsigned						flags;
	int								reloc;
};

extern uintptr_t	user_addr;
extern uintptr_t	user_addr_end;

/* __SRCVERSION("pmm.h $Rev: 153052 $"); */
