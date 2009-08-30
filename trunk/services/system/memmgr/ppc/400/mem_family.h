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

//
// PPC 400 specific memory manager definitions.
//
#include <ppc/400cpu.h>

#define PPC_INIT_ASID(adp)	1

#define PTE_PERMS(ptep)			(*(ptep) & 0xfff)
#define PTE_PRESENT(ptep)		(*(ptep) & (0xfff & ~PPC400_TLBLO_I))
#define PTE_CACHEABLE(ptep)		((*(ptep) & PPC400_TLBLO_I)==0)
#define PTE_READABLE(ptep)		(1)
#define PTE_EXECUTABLE(ptep)	(*(ptep) & PPC400_TLBLO_EX)
#define PTE_WRITEABLE(ptep)		(*(ptep) & PPC400_TLBLO_WR)

#define PTE_PGSIZE(ptep)		(__PAGESIZE)
#define PTE_PADDR(ptep)			(*(ptep) & ~(__PAGESIZE-1))

#define L1_SHIFT	22

typedef uint32_t	pte_t;

/* __SRCVERSION("mem_family.h $Rev: 153052 $"); */
