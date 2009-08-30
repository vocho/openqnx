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
// PPC 600 specific memory manager definitions.
//

#undef  CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES
#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES	VPS_HIGHUSAGE

#define PPC_INIT_ASID(adp) \
	(fam_pte_asid_alloc(adp), (adp->cpu.asid != PPC_INVALID_ASID))

#define PTE_PERMS(ptep)			(*(ptep) & 0xfff)
#define PTE_PRESENT(ptep)		(*(ptep) & (0xfff & ~PPC_TLBLO_I))
#define PTE_CACHEABLE(ptep) 	((*(ptep) & PPC_TLBLO_I)==0)
#define PTE_READABLE(ptep) 		(*(ptep) & PPC_TLBLO_PP_MASK)
#define PTE_EXECUTABLE(ptep)	(*(ptep) & PPC_TLBLO_PP_MASK)
#define PTE_WRITEABLE(ptep)		((*(ptep) & PPC_TLBLO_PP_MASK) == PPC_TLBLO_PP_RW)

#define PTE_PGSIZE(ptep)		(__PAGESIZE)
#define PTE_PADDR(ptep) \
		((paddr_t)(*(ptep) & ~(__PAGESIZE-1)) \
	 	| ((paddr_t)(*(ptep) & 0x00000e00) << 24) \
		| ((paddr_t)(*(ptep) & 0x4) << 30))

#define L1_SHIFT	22	

typedef uint32_t	pte_t;


/*
 * Implementation note for MSG_XFER_SET_SR
 * PPC documentation (Green Book - Ch 2, Tbl 2-19) indicates that the "mtsr"
 * operation be bracketed by context synchronizing instructions (isync) in
 * order to ensure data access synchronization. A preceeding "isync" is
 * not required for instruction access synchronization. Because of the
 * performance impact of the "isync" operation and the current use of this
 * macro (relative to other instructions in the stream that perform data
 * accesses), it has been decided that it is safe to omit the preceeding
 * "isync" and still obtain correctness of the intended code sequence.
 * If the use or placement of this macro changes, this assumption should
 * be reevaluated.
 * - mk
*/
#define MSG_XFER_SET_SR(sr, adp, base)	{		\
	uint32_t	__vsid = 0x70000000 | (adp)->cpu.asid | ((base & 0xf0000000) >> 8);	\
	__asm__ __volatile__( 		\
/*	"isync;"	*/				\
	"mtsr %1,%0;"				\
	"isync;"					\
	: : "b" (__vsid), "i" (sr));\
	}

/* __SRCVERSION("mem_family.h $Rev: 164196 $"); */
