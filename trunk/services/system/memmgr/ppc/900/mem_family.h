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
// PPC 900 specific memory manager definitions.
//
#define PPC_INIT_ASID(adp) \
	(fam_pte_asid_alloc(adp), (adp->cpu.asid != PPC_INVALID_ASID))

#define PTE_PERMS(ptep)			((unsigned)*(ptep) & 0xfff)
#define PTE_PRESENT(ptep)		((unsigned)*(ptep) & (0xfff & ~PPC_TLBLO_I))
#define PTE_CACHEABLE(ptep) 	(((unsigned)*(ptep) & PPC_TLBLO_I)==0)
#define PTE_READABLE(ptep) 		((unsigned)*(ptep) & PPC_TLBLO_PP_MASK)
#define PTE_EXECUTABLE(ptep)	(((unsigned)*(ptep) & PPC64_TLBLO_N)==0)
#define PTE_WRITEABLE(ptep)		(((unsigned)*(ptep) & PPC_TLBLO_PP_MASK) == PPC_TLBLO_PP_RW)

#define PTE_PGSIZE(ptep)		(__PAGESIZE)
#define PTE_PADDR(ptep) 		((paddr_t)(*(ptep) & ~(paddr_t)(__PAGESIZE-1)))

#define L1_SHIFT	21

typedef uint64_t	pte_t;

//YYY: Have to do SMP adjust for top 256M vaddrs
#define MSG_XFER_SET_SR(sr, adp, base)	{									\
	uint32_t	__vsid;														\
	unsigned	idx;														\
																			\
	/* SLIBIE to force invalidation of [I,D]-ERAT's */                      \
	ppc_slbie((sr) << PPC64_SLB1_ESID_SHIFT);								\
	idx = (base) >> 28;														\
	/* SMP adjust for high 256M so that cpupage can be kept seperate */		\
	if(idx == 0xf) idx += RUNCPU;											\
	__vsid = (((adp)->cpu.asid + idx) << PPC64_SLB0_VSID_SHIFT)				\
				| (PPC64_SLB0_KS|PPC64_SLB0_KP|PPC64_SLB0_N);				\
	ppc_slbmte(__vsid, (((sr)<<PPC64_SLB1_ESID_SHIFT) | (sr)) | PPC64_SLB1_V);\
	ppc_isync();															\
	}

/* __SRCVERSION("mem_family.h $Rev: 153052 $"); */
