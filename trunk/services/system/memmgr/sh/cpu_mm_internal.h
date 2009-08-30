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



// Low and high ranges of memory that the system (kernel/proc) can quickly
// get at.
#define CPU_SYSTEM_PADDR_START	0
#define CPU_SYSTEM_PADDR_END	(SH_P1SIZE-1)

// Whether the system can _only_ access paddrs in the above range.
#define CPU_SYSTEM_PADDR_MUST	1

// Whether the system has memory colours
#define CPU_SYSTEM_HAVE_COLOURS	1

// Whether the system supports multiple page sizes
#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES  VPS_AUTO

#define CPU_1TO1_IS_VADDR(v)	CPU_VADDR_IN_RANGE(v)
#define CPU_1TO1_IS_PADDR(p)	((p) <= CPU_SYSTEM_PADDR_END)
#define CPU_1TO1_VADDR_BIAS		SH_P1BASE

#define CPU_SO_VADDR_START 		0x78200000
#define CPU_SHMEM_VADDR_START	0x38100000

#define CPU_USER_VADDR_START	SH_P0BASE
#define CPU_USER_VADDR_END	VM_USER_SPACE_BOUNDRY - 1

/* addr: 4 bytes boundry; len:>=16 and multiply of 16 bytes */
#define CPU_ZERO_PAGE(addr, len, mm) ({ 			\
	unsigned _addr=(uintptr_t)addr, _len=(unsigned)len;	\
		__asm__ __volatile__(							\
		"1:;" 											\
		"mov.l	%2,@(0,%0);"							\
		"add	#-16,%1;"								\
		"mov.l	%2,@(4,%0);" 							\
		"tst	%1,%1;"									\
		"mov.l	%2,@(8,%0);" 							\
		"mov.l	%2,@(12,%0);" 							\
		"bf.s	1b;"									\
		"add	#16,%0;"								\
		 :"=&r" (_addr), "=&r" (_len)					\
		 :"r" (0), "0" (_addr), "1" (_len)				\
		 :"cc", "memory");})


struct cpu_mm_aspace {
	unsigned			asid;
	uint32_t			**pgdir;
	struct pa_quantum	*l2_list;
	volatile unsigned	asid_flush;
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// CPU specific definitions...
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//RUSH3: Check usage of these macros later...
#define PRESENT(pte)	((pte) & SH_CCN_PTEL_V)

#define REAL_ADDR(pte)	((pte) & ~(__PAGESIZE-1) )

#define	L1PAGEIDX(vaddr)	((uintptr_t)(vaddr) >> 22)
#define	L2PAGEIDX(vaddr)	(((uintptr_t)(vaddr) >> 12) & 0x3ff)

#define IS_CACHEABLE(pte)		((pte) & SH_CCN_PTEL_C)
#define IS_PAGEREADABLE(pte)	((pte) & SH_CCN_PTEL_PR(3))
#define IS_PAGEWRITEABLE(pte)	((pte) & SH_CCN_PTEL_PR(1))

#define ISNOT_PRESENT_READABLE(pte)	\
	( ((pte) & (SH_CCN_PTEL_PR(2) | SH_CCN_PTEL_V)) != (SH_CCN_PTEL_PR(2) | SH_CCN_PTEL_V) )
#define ISNOT_PRESENT_WRITEABLE(pte)	\
	( ((pte) & (SH_CCN_PTEL_PR(1) | SH_CCN_PTEL_V)) != (SH_CCN_PTEL_PR(1) | SH_CCN_PTEL_V) )

#define NUM_TLB_RESERVED	2



static inline void __attribute__((__unused__))
_p2_out32(unsigned __dst, unsigned __new) {
	unsigned	amask = 0xa0000000;
	__asm__ __volatile__(
		"	mova	1f,r0;"
		"	bra		2f;"
		"	nop;"
		".align 2;"
		"1:;"
		"	mov.l	%2,@%1;"
		"	nop;nop;nop;nop;"   // h/w manual says we can't branch out of the p2 area for 8
		"	nop;nop;nop;nop;"   // instructions after the mov (7751 h/w manual, section 3.7)
		"	rts;"
		"	nop;"
		"2:;"
		"	or		%0,r0;"
		"	jsr		@r0;"
		"	nop;"
		: 
		: "r" (amask), "r" (__dst), "r" (__new)
		: "r0", "pr", "memory"
	);
}

static inline unsigned __attribute__((__unused__))
_p2_in32(unsigned __src) {
	unsigned	amask = 0xa0000000, ret;
	__asm__ __volatile__(
		"	mova	1f,r0;"
		"	bra		2f;"
		"	nop;"
		".align 2;"
		"1:;"
		"	mov.l	@%2,%0;"
		"	nop;nop;nop;nop;"  // h/w manual says we can't branch out of the p2 area for 8
		"	nop;nop;nop;nop;"  // instructions after the mov (7751 h/w manual, section 3.7)
		"	rts;"
		"	nop;"
		"2:;"
		"	or		%1,r0;"
		"	jsr		@r0;"
		"	nop;"
		:"=&r" (ret)
		: "r" (amask), "r" (__src)
		: "r0", "pr", "memory"
	);

	return ret;
}

// The method required to update the TLBs through the memory mapped region
// differs between SH-4 and SH-4a, so we use function pointers that are assigned
// early in the procnto initialization for this functionality.
EXTERN unsigned (*sh_tlb_in32)(unsigned p);
EXTERN void (*sh_tlb_out32)(unsigned p, unsigned v);

#define MAX_XFER_SLOTS	10
#define KERN_XFER_SLOT_SIZE     (MEG(8))

struct xfer_map {
	PROCESS		*prp;
	unsigned	diff0;
	unsigned	size0;
	unsigned	diff1;
	unsigned	size1;
};

#define PERMANENT_MAP_START	(SH_P4BASE - MEG(32))
#define PERMANENT_MAP_END	(SH_P4BASE - 1)


EXTERN ADDRESS *asid_map[VM_ASID_BOUNDARY+1];
EXTERN struct xfer_map	xfer_map[PROCESSORS_MAX];

void 	mem_page_cache_invalidate_phy(paddr_t, size_t);
void	tlb_flush_all(void);
void	tlb_flush_va(ADDRESS *adp, uintptr_t start, uintptr_t end);
void	tlb_flush_asid(unsigned asid);
void 	tlb_flush_entry(ADDRESS *adp, unsigned entry); 
void    perm_map_init(void);
void	cpu_pte_init(void);
#ifdef	VARIANT_smp
void	smp_tlb_sync(PROCESS *);
extern struct intrspin	asid_spin;
#else
#define	smp_tlb_sync(p)
#endif


/* __SRCVERSION("cpu_mm_internal.h $Rev: 204740 $"); */
