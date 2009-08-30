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

/*
 * ARMv4 specific memmgr implementation
 */

// Whether the system has memory colours
#define CPU_SYSTEM_HAVE_COLOURS	0

/*
 * ARM FCSE MMU limits address space to 32MB
 */
#define	CPU_USER_VADDR_START	0
#define	CPU_USER_VADDR_END		(USER_SIZE-1)
#define CPU_GBL_VADDR_START		0x80000000
#define CPU_GBL_VADDR_END		0xbfffffff
#define	CPU_SO_VADDR_START		0x01000000
#define	CPU_SHMEM_VADDR_START	0x01800000

/*
 * Virtually indexed cache needs extra maintainence.
 * CPU_CACHE_CONTROL() needs to be aware of FCSE operation.
 */
#define	CPU_1TO1_FLUSH(v, l)			CacheControl((void *)(v), l, MS_INVALIDATE|MS_INVALIDATE_ICACHE)
#define CPU_CACHE_CONTROL(a, d, l, f)	cpu_cache_control((a), (d), (l), (f))

struct arm_gbl_map;

struct cpu_mm_aspace {
	uint8_t				asid;			// assigned by vmm_mcreate()
	volatile uint8_t	domain;			// can be changed by context switch
	int16_t				gbl_swtch;		// has protected global mappings
	struct arm_gbl_map	*gbl_map;		// list of global mappings
	void				*pgdir;			// dummy field
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// CPU specific definitions...
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

extern void	cpu_cache_control(ADDRESS *__adp, void *__s, size_t __l, unsigned __f);

// L1 entry for page table manipulation
#define	VTOL1PT(va)				(L1_table + (((va) >> 20) & ~3))

// L1 entry for section containing va
#define	VTOL1SC(va)				(L1_table + ((va) >> 20))

// Compatability macros to allow source sharing with V6
#define	V6_USER_SPACE(va)		0
#define	KTOL1SC(va)				VTOL1SC(va)
#define	UTOL1SC(va)				((unsigned *)0)

#define VX_CACHE_COLOURS()	1
#define VX_XFER_INIT()

/*
 * Macros to manipulate L1 descriptor domain field
 */
#define	PTP_DOMAIN_MASK			(15 << 5)
#define	PTP_DOMAIN(pt)			(((pt) >> 5) & 15)
#define	PTP_DOMAIN_CLR(pt)		((pt) &= ~PTP_DOMAIN_MASK)
#define	PTP_DOMAIN_SET(pt, d)	((pt) = ((pt) & ~PTP_DOMAIN_MASK) | ((d) << 5))

/*
 * User address space is 0 - 32MB, remapped by CP15 ProcessID
 */
#define USER_BASE		0x00000000u
#define USER_SIZE		0x02000000u		// 32MB total

// convert ASID to virtual address offset
#define	MVA_BASE(a)	((a) << 25)	

/*
 * Global mapping support
 */
struct arm_gbl_map {
	uint16_t			idx;	// L1 table index: uses 12 bits
	int16_t				cnt;	// number of pages mapped
	struct arm_gbl_map	*next;	// next mapping in list
};

#define	ARM_GBL_MAP_GLOBAL	(1<<15)	// SHMCTL_GLOBAL mapping
#define	ARM_GBL_MAP_IDX(m)	((m)->idx & ~ARM_GBL_MAP_GLOBAL)

#define	ARM_GBL_MAPPING(v)		((v) >= CPU_GBL_VADDR_START && (v) <= CPU_GBL_VADDR_END)
#define	ARM_GBL_PRIVILEGED(f)	((f) & SHMCTL_PRIV)
#define	ARM_GBL_PROTECTED(f)	(((f) & (SHMCTL_PRIV|SHMCTL_LOWERPROT)) == 0)

/*
 * ARMv4/ARMv5 use ARM_PTE_W to indicate writability.
 */
#define	ARM_PTE_WRITABLE(pte)	(((pte) & ARM_XSP_PROT(ARM_PTE_RW)) == ARM_XSP_PROT(ARM_PTE_RW))

/*
 * PTE_OP_BAD uses invalid pte with ARM_PTE_RW set
 */
#define	ARM_PTE_BAD				ARM_XSP_PROT(ARM_PTE_RW)

extern PROCESS		*xfer_prp;
extern ptrdiff_t	xfer_diff;

extern void			domain_free(ADDRESS *);
extern void			arm_gbl_update(ADDRESS *, int);

/* __SRCVERSION("vx_mm_internal.h $Rev: 153052 $"); */
