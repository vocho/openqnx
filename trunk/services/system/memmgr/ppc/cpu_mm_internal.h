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
#define CPU_SYSTEM_PADDR_END	(VM_KERN_LOW_SIZE - 1)

// Whether the system can _only_ access paddrs in the above range.
#define CPU_SYSTEM_PADDR_MUST	1

// Whether the system has memory colours
#define CPU_SYSTEM_HAVE_COLOURS	0

// Whether we support variable pagesizes
// Overridden for Book E, 600's
#define CPU_SYSTEM_HAVE_MULTIPLE_PAGESIZES	VPS_NONE 

// Starting vaddrs for shared objects and shared memory
#define CPU_SO_VADDR_START 		0xfe300000
#define CPU_SHMEM_VADDR_START	0x80100000
#define CPU_USER_VADDR_START	VM_USER_SPACE_BOUNDRY
#define CPU_USER_VADDR_END		(VM_USER_SPACE_BOUNDRY+VM_USER_SPACE_SIZE-1)

// Define the 1-to-1 region
#define CPU_1TO1_VADDR_BIAS		0
#define CPU_1TO1_IS_VADDR(v)	((v) < VM_KERN_LOW_SIZE)
#define CPU_1TO1_IS_PADDR(p)	((p) < VM_KERN_LOW_SIZE)

#define CPU_ZERO_PAGE(d,l,mm) 	zero_page((d), (l), (mm))


// PPC family specific definitions. Have to include up here so
// we can get the "pte_t" type definition for cpu_mm_aspace
#include "mem_family.h"

struct cpu_mm_aspace {
	unsigned			asid;
	pte_t				**pgdir;
	struct pa_quantum	*l2_list;
#if defined(VARIANT_600)
	struct {
		uint32_t		lo;
		uint32_t		up;
	}					bat[8];
	unsigned			nx_state;
#elif defined(VARIANT_booke)
	intrspin_t			slock;
	unsigned			pending_wire_delete;
	volatile unsigned	pending_wire_sync;
	volatile unsigned	pending_asid_flush;
	struct wire_entry	*wires;
#endif	
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// CPU specific definitions...
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define L2_SHIFT	12

#define	L1PAGEIDX(vaddr)	((uintptr_t)(vaddr) >> L1_SHIFT)
#define	L2PAGEIDX(vaddr)	(((uintptr_t)(vaddr) >> L2_SHIFT) & ((1 << (L1_SHIFT-L2_SHIFT))-1))

#define PDE_SIZE (1 << L1_SHIFT)

#define PPC_INVALID_ASID		((uint32_t)~0U)

#define PDE_ADDR(p)	((pte_t *)(ADDR_PAGE((uintptr_t)(p))))


/*
 * Function to zero pages. Uses either word moves or dcbz;
 * we can use these because we know what the page alignment and size is.
 */

#define ZP_NOCACHE		PROT_NOCACHE // Must use same bit
// We can reuse the PG_* bits because we know they'll never be on in the 
// mmap_flags field  (output only from vmm_mapinfo)
#define ZP_CACHE_PURGE	PG_MODIFIED
#define ZP_CACHE_OFF	PG_REFERENCED
#define ZP_DCBZ_BAD		PG_HWMAPPED

extern unsigned					zp_flags;
struct mm_map;
extern void zero_page(void *s, size_t len, struct mm_map *);

struct mm_aspace;
extern void fam_pte_init(int phase);
extern void fam_pte_flush_all(void);
extern void fam_pte_mapping_add(uintptr_t vaddr, paddr_t paddr, unsigned prot, unsigned flags);
extern void fam_pte_mapping_del(struct mm_aspace *adp, uintptr_t vaddr, unsigned size);
extern void fam_pte_asid_alloc(struct mm_aspace *adp);
extern void fam_pte_asid_release(struct mm_aspace *adp);


#define PPC_SPECIAL_MASK	(PPC_SPECIAL_W|PPC_SPECIAL_I|PPC_SPECIAL_M|PPC_SPECIAL_G|PPC_SPECIAL_E)


/*	AM Note:
	These xfer_* variables are actually used by PPC600 vm600.s (which has weak refs)
*/
#define NUM_XFER_MAPPINGS 2
extern PROCESS		*xfer_prp;
extern uintptr_t	xfer_diff[NUM_XFER_MAPPINGS];
extern uintptr_t	xfer_lastaddr[NUM_XFER_MAPPINGS];
extern unsigned		xfer_rotor;

/* __SRCVERSION("cpu_mm_internal.h $Rev: 199396 $"); */
