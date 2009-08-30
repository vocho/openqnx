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
#define CPU_SYSTEM_PADDR_END	(MIPS_R4K_K0SIZE-1)

// Whether the system can _only_ access paddrs in the above range.
#define CPU_SYSTEM_PADDR_MUST	1

// Whether the system has memory colours, variable sized pages
#if defined(VARIANT_r3k)
	#define CPU_SYSTEM_HAVE_COLOURS	0
	#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES	VPS_NONE
#else
	#define CPU_SYSTEM_HAVE_COLOURS	1
	#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES	VPS_AUTO
#endif


#define CPU_USER_VADDR_START	MIPS_R4K_KUBASE
#define CPU_USER_VADDR_END		(MIPS_R4K_KUBASE+MIPS_R4K_KUSIZE-1)

#define CPU_1TO1_VADDR_BIAS		MIPS_R4K_K0BASE
#define CPU_1TO1_IS_VADDR(v)	MIPS_IS_KSEG0(v)
#define CPU_1TO1_IS_PADDR(p)	((p) < MIPS_R4K_K0SIZE)

#define CPU_SO_VADDR_START		0x78200000
#define CPU_SHMEM_VADDR_START	0x38100000

struct cpu_mm_aspace {
	unsigned				asid[PROCESSORS_MAX];
	volatile unsigned		pending_asid_purges;
	struct pa_quantum		*l2_list;
	struct pte				**pgdir;
	intrspin_t				slock;
	unsigned				pending_wire_deletes;
	volatile unsigned		pending_wire_syncs;
	struct wire_entry		*wires;
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// CPU specific definitions...
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

typedef struct pte pte_t;


#include <mips/vm.h>

/*
 * TLB macros
 */
#if defined(VARIANT_r3k)
	#define	TLB_GLOBAL			MIPS3K_TLB_GLOBAL
	#define	TLB_WRITE			MIPS3K_TLB_WRITE
	#define	TLB_VALID			MIPS3K_TLB_VALID
	#define TLB_NOCACHE 		MIPS3K_TLB_NOCACHE
	#define TLB_CACHEABLE		0

	#define PTE_PADDR(pte)		((paddr_t)((pte) & TLB_LO_PFNMASK))
	#define PTE_CACHEABLE(pte)	(!((pte) & TLB_NOCACHE))

	#define TLB_HI_ASIDSHIFT	MIPS3K_TLB_HI_ASIDSHIFT
	#define TLB_HI_ASIDMASK		MIPS3K_TLB_HI_ASIDMASK
	#define TLB_LO_PFNMASK		MIPS3K_TLB_LO_PFNMASK

	#define TLB_MAX_ASID		MIPS3K_TLB_MAX_ASID
	#define TLB_IDX_SHIFT		MIPS3K_TLB_INX_INDEXSHIFT

	#define TLB_MAX_SPECIAL		0

	#define PGMASK_TO_PGSIZE(m) 	(((m) | (__PAGESIZE-1))+1)
	#define VPN_MASK(m) 			(~((m) | (__PAGESIZE-1)))
#else
	#define	TLB_GLOBAL			MIPS_TLB_GLOBAL
	#define	TLB_WRITE			MIPS_TLB_WRITE
	#define	TLB_VALID			MIPS_TLB_VALID
	#define TLB_NOCACHE 		(MIPS_TLB_UNCACHED << MIPS_TLB_LO_CSHIFT)
	#define TLB_CACHEABLE		pt_cacheable_attr

	#define PTE_PADDR(pte)		((((paddr_t)((uint32_t)(pte) & TLB_LO_PFNMASK)) << pfn_topshift) | ((pte & 0x80000000) ? 0xe000000000ULL : 0))
	#define PTE_CACHEABLE(pte)	(((pte) & MIPS_TLB_LO_CMASK) != TLB_NOCACHE)

	#define TLB_HI_ASIDSHIFT	MIPS_TLB_HI_ASIDSHIFT
	#define TLB_HI_ASIDMASK		MIPS_TLB_HI_ASIDMASK
	#define TLB_LO_PFNMASK		(MIPS_TLB_LO_PFNMASK|0xc0000000)

	#define TLB_MAX_ASID		MIPS_TLB_MAX_ASID
	#define TLB_IDX_SHIFT		MIPS_TLB_INX_INDEXSHIFT

	#define TLB_MAX_SPECIAL		(MIPS_TLB_LO_CMASK >> MIPS_TLB_LO_CSHIFT)

	#define PGMASK_TO_PGSIZE(m) 	((((m) | (__PAGESIZE*2-1))+1) >> 1)
	#define VPN_MASK(m) 			(~((m) | (__PAGESIZE*2-1)))
#endif
#define PTE_PRESENT(p)		((p) & ~(TLB_GLOBAL|TLB_WRITE))


#define KERN_XFER_SLOT_SIZE     (1 << PT_L1SHIFT)

struct xfer_map {
	uintptr_t       xferbase;
	ptrdiff_t       diff;
	PROCESS         *prp;
	unsigned		slots;
};

struct asid_mapping {
	unsigned    rotor;
	ADDRESS     *map[TLB_MAX_ASID+1];
};

struct mm_pte_manipulate;

EXTERN unsigned				pfn_topshift;
EXTERN unsigned 			pt_cacheable_attr;
EXTERN unsigned				num_tlbs;
EXTERN unsigned				dcache_size;
EXTERN unsigned				dcache_lines;
EXTERN unsigned 			dcache_lsize;
EXTERN unsigned 			dcache_lines_per_page;
EXTERN unsigned				icache_size;
EXTERN unsigned				icache_lines;
EXTERN unsigned				icache_lsize;
EXTERN unsigned 			icache_lines_per_page;
EXTERN unsigned 			icache_flags;
EXTERN uintptr_t			kern_xfer_base;
EXTERN uintptr_t			kern_xfer_end;
EXTERN struct asid_mapping	*gbl_asid_map[PROCESSORS_MAX];
EXTERN struct xfer_map		*xfer_tbl;
EXTERN unsigned				pgmask_4k;

/*
 * Note: the following definition only makes sense
 * if 0 is NOT used as an asid for a process.
 */
//RUSH3: Try allowing zero as a valid ASID
#define VALID_ASID(x)		(((x) > 0) && ((x) <= TLB_MAX_ASID))

struct pa_quantum;

extern void r4k_purge_icache_full(void);
extern void r4k_purge_dcache_page(void *);
extern void r4k_flush_dcache_page(void *);
extern void r4k_purge_dcache_page_hitwa(unsigned);
extern void r4k_flush_dcache_page_hitwa(unsigned);

extern unsigned		discover_num_tlb(unsigned);
extern uintptr_t	colour_init(uintptr_t, unsigned);
extern void			colour_operation(void (*)(void *), unsigned, paddr_t);
extern void 		clean_init(void);
extern uintptr_t	perm_map_init(void);
extern uintptr_t	xfer_init(uintptr_t, uintptr_t);
extern void			xfer_cache_kill(ADDRESS *);	
extern void			pgszlist_init(uintptr_t);
extern void			wire_init(void);
extern void			wire_sync(ADDRESS *);
extern void			wire_check(struct mm_pte_manipulate *);
extern void			wire_mcreate(PROCESS *);

#define PENDING_PURGE_ADDRESS	((ADDRESS *)~0)

// We can't support the ptehack anymore because it modifies the paddr
// of pte entries and since we now support >4G paddr's, the bit that it
// turns on gets returned by the PTE_PADDR() macro. That bit gets fed
// into a pa_free_paddr() over in mem_virtual.c (in the alloc_entry_free
// function) and since it's a physical address that doesn't 'exist' on
// the system, we end up calling pa_free() with a NULL pa_quantum ptr.
//RUSH3: The above comment might not be true anymore, since we're keeping
//RUSH3: the pa_quantum pointers around. Something to think about later.
#undef PTEHACK_SUPPORT

/* __SRCVERSION("cpu_mm_internal.h $Rev: 165175 $"); */
