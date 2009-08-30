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
//
#define CPU_SYSTEM_PADDR_START	0
#define CPU_SYSTEM_PADDR_END	((256*1024*1024)-1)

// Whether the system can _only_ access paddrs in the above range.
#define CPU_SYSTEM_PADDR_MUST	0

// Whether the system has memory colours
#define CPU_SYSTEM_HAVE_COLOURS	0

// Whether the system supports multiple page sizes
#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES	((__cpu_flags & X86_CPU_PSE) ? VPS_AUTO : VPS_NONE)

//Identify 1-to-1 mapping vaddrs & paddrs
#define CPU_1TO1_VADDR_BIAS		0xe0000000u
#define CPU_1TO1_IS_VADDR(v)	((__cpu_flags & X86_CPU_PSE) && ((v) >= CPU_1TO1_VADDR_BIAS) && ((v) <= (CPU_1TO1_VADDR_BIAS+CPU_SYSTEM_PADDR_END)))
#define CPU_1TO1_IS_PADDR(p)	((__cpu_flags & X86_CPU_PSE) && ((p) <= CPU_SYSTEM_PADDR_END))

// Virtual address regions for user programs
#define CPU_USER_VADDR_START	0x00000000u
#define CPU_USER_VADDR_END		0xbfffffffu
#define CPU_SO_VADDR_START 		0xb8200000u
#define CPU_SHMEM_VADDR_START	0x40100000u

#define CPU_ZERO_PAGE(d, l, f)	(_zero_dwords(d, (unsigned)(l) >> 2))

#define CPU_FAULT_ON_WRITE_WORKS	(__cpu_flags & X86_CPU_WP)

struct cpu_mm_aspace {
	struct cpu_mm_aspace	*next;
	struct cpu_mm_aspace	**prev;
	union pxe				*pgdir;
	struct pa_quantum		*l2_list;
	paddr32_t				ptroot_paddr;
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// CPU specific definitions...
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

extern void pgtbl_init(void);

typedef union pxe {
	uint32_t		pxe32;
	uint64_t		pxe64;
} pxe_t; // used for both page directory entries and page table entries.

struct xfer_slots {
	uintptr_t						addr;	// Address of last slot used
	uintptr_t						first;	// First slot available to cpu
	uintptr_t						last;	// Last slot plus size for cpu
	uintptr_t						spare;	
	ptrdiff_t						diff;	// diff from mapping to real vaddr
	volatile PROCESS				*prp;
	uintptr_t						base0;	// Address of second to last slot used
	ptrdiff_t						diff0;	// diff from mapping to real vaddr
};

extern int						pae_enabled;
extern unsigned					pd_bits;
extern unsigned					pxe_bits;
extern uintptr_t				pde_map;
extern struct xfer_slots		*xfer_slot;


/* This is used when PAE is disabled */
/* To make these work the last entry of the page directory points to itself */

#define PXE1_BITS		 2				// 2 bits of addr for each tbl entry
#define PD1_BITS		22				// 22 bits in page directory

#define PG1_REMAP       1023
#define PG1_PDIRADDR    (PG1_REMAP << PD1_BITS | PG1_REMAP << 12)							// Pointer to start of page directory
#define PG1_PTEADDR 	(PG1_REMAP << PD1_BITS)											// Pointer to first page table

#define V1TOPDIRP(v)    ((uint32_t *)(PG1_PDIRADDR | (((uint32_t)(v))>>20&~3)))		// Pointer to page directory entry
#define V1TOPTEP(v)     ((uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&~3)))		// Pointer to page table entry
#define V1TOPTP(v)      ((uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&0x3ff000))) // Pointer to start of page table


/* This is used when PAE is enabled */
/* To make these work the last 4 entries of the PDPT point to 4 pages within a 16k page directory */

#define PXE2_BITS		 3				// 3 bits of addr for each tbl entry
#define PD2_BITS		21				// 21 bits in page directory

#define PG2_PDIRADDR    0xffffc000		// Pointer to start of page directory
#define PG2_PTEADDR     0xff800000		// Pointer to first page table

#define V2TOPDIRP(v)    ((uint64_t *)(PG2_PDIRADDR | (((uint32_t)(v))>>18&~7)))		// Pointer to page directory entry
#define V2TOPTEP(v)     ((uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&~7)))		// Pointer to page table entry
#define V2TOPTP(v)      ((uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&0x7ff000)))	// Pointer to start of page table


/* Combo macros that do the right thing whether PAE is enabled or not */

#define PXE_ADD(p,o) 		((pxe_t *)((uintptr_t)(p) + (o)))
#define	PXE_GET(p)			(pae_enabled ? (p)->pxe64 : (p)->pxe32)
#define PXE_SET(p,v)		(pae_enabled ? ((p)->pxe64 = (v)) : ((p)->pxe32 = (v)))
#if defined(__LITTLEENDIAN__)
	#define PXE_GET_FLAGS(p)	((p)->pxe32)
	#define PXE_SET_FLAGS(p,m,s) ((p)->pxe32 = ((p)->pxe32 & (m)) | (s))
#elif defined(__BIGENDIAN__)
	#error not supported
#else
	#error Endianness not set
#endif

#define GENERIC_VTOPDIRP(r,v) PXE_ADD(r,((v) >> pd_bits) << pxe_bits)
#define VTOPDIRP(v)		(pae_enabled ? (pxe_t *)V2TOPDIRP(v) : (pxe_t *)V1TOPDIRP(v))
#define VTOPTEP(v)		(pae_enabled ? (pxe_t *)V2TOPTEP(v)  : (pxe_t *)V1TOPTEP(v))
#define VTOPTP(v)		(pae_enabled ? (pxe_t *)V2TOPTP(v)   : (pxe_t *)V1TOPTP(v))


#define PTE_PADDR_BITS	(~((uint64_t)(0xfff|X86_PTE_NX)))


#define SYSADDR_BASE		0xc0000000u
#define MAP_BASE			SYSADDR_BASE
#define MAP_SIZE			0x10000000u
#define L2MAP_BASE			(MAP_BASE+MAP_SIZE)
#define PROCMEM_BASE		(L2MAP_BASE+(4*1024*1024))

void			x86_init_mtrr(void);
int				x86_set_mtrr(paddr_t start, unsigned size, unsigned flags, unsigned op);

#if defined(__WATCOMC__)

int _zero_dwords(void *ptr, int num);
#pragma aux _zero_dwords = \
			"cld" \
			"mov eax,0" \
			"rep stosd" \
			parm [edi] [ecx] \
			modify exact [eax ecx edi];

#elif (defined (__GNUC__) || defined(__ICC))

#define _zero_dwords(__ptr, __num) ({		\
	__asm__ __volatile__ ( 					\
			"movl  %0,%%edi \n"	\
			"movl  %1,%%ecx \n"	\
			"cld \n" 						\
			"movl $0,%%eax \n"				\
			"rep \n"						\
			"stosl \n"						\
			:								\
			: "r"(__ptr), "r"(__num)		\
			: "eax", "ecx", "edi" );		\
			})
#else

	#error Compiler not supported

#endif

/* __SRCVERSION("cpu_mm_internal.h $Rev: 201493 $"); */
