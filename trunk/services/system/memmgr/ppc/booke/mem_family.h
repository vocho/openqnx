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
// PPC Book E specific memory manager definitions.
//
#include <ppc/bookecpu.h>

extern unsigned	vps_state;

#undef  CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES
#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES	vps_state

#define PPC_INIT_ASID(adp)	1

#define PTE_PERMS(ptep)			((ptep)->flags & ~PPCBKE_PTE_PGSZ_MASK)
#define PTE_PRESENT(ptep)		((ptep)->flags & (0xff & ~PPCBKE_PTE_ATTR_I))
#define PTE_CACHEABLE(ptep)		(((ptep)->flags & PPCBKE_PTE_ATTR_I)==0)
#define PTE_READABLE(ptep)		((ptep)->flags & PPCBKE_PTE_ACCESS_R)
#define PTE_EXECUTABLE(ptep)	((ptep)->flags & PPCBKE_PTE_ACCESS_X)
#define PTE_WRITEABLE(ptep)		((ptep)->flags & PPCBKE_PTE_ACCESS_W)

// actual pagesize calc is "0x400 << (tlbsize*2)", but we'll play some
// games to get more efficient code
#define PTE_PGSIZE(ptep)	\
	(0x400 << (((ptep)->flags >> (PPCBKE_PTE_PGSZ_SHIFT-1)) & (PPCBKE_PTE_PGSZ_MASK >> (PPCBKE_PTE_PGSZ_SHIFT-1))))
#define PTE_PADDR(ptep)	\
	(((ptep)->paddr & ~0xfff) | ((paddr_t)((ptep)->paddr & 0xfff) << 32))

#define L1_SHIFT	21

typedef struct ppcbke_pte	pte_t;
struct mm_pte_manipulate;

extern void			wire_init(void);
extern void			wire_check(struct mm_pte_manipulate *);

/* __SRCVERSION("mem_family.h $Rev: 164196 $"); */
