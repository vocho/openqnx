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
 * ARMv6 specific memmgr implementation
 */

// Whether the system has memory colours
#define CPU_SYSTEM_HAVE_COLOURS	1

/*
 * Virtual address regions for user programs
 */
#define	CPU_USER_VADDR_START	0
#define	CPU_USER_VADDR_END		0x7fffffff
#define	CPU_SO_VADDR_START		0x78000000
#define	CPU_SHMEM_VADDR_START	0x28000000

struct cpu_mm_aspace {
	uint8_t				asid;			// assigned by vmm_mcreate()
	void				*pgdir;			// dummy field
	struct pa_quantum	*l1_pq;			// quantum for L1 (8K size table)
	struct pa_quantum	*l2_pq;			// quantum for "page directory" page
	pte_t				l1_pte;			// pte for L1 table
	unsigned			ttbr0;			// L1 table value for ttbr0 register
	pte_t				l2_pte;			// pte for "page directory" page
	ptp_t				l2_ptp;			// L1 descriptors for "page directory"
	struct pa_quantum	*l2_list;		// L2 page tables
	unsigned			asid_flush;		// cpu's requiring TLB ASID flush
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// CPU specific definitions...
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

struct xfer_slot {
	volatile PROCESS	*prp;
	unsigned			diff0;
	unsigned			size0;
	unsigned			diff1;
	unsigned			size1;
};
extern struct xfer_slot	*xfer_slots;

extern pte_t				*L2_vaddr;

/*
 * Mask off physical address of the 8K TTBR0
 * This is required to work around some chip errata where reading the
 * TTBR0 register doesn't return all the control bits
 */
#define	TTBR0_MASK	8191

/*
 * Check if address is in user address space
 */
#define	V6_USER_SPACE(va)	((unsigned)(va) < (unsigned)CPU_USER_VADDR_END)

/*
 * L1 entry for page table manipulation (4K of L2 -> 4MB of address space)
 */
#define	KTOL1PT(va)				(L1_table + (((unsigned)(va) >> 20) & ~3))
#define	UTOL1PT(va)				((ptp_t *)ARM_V6_USER_L1 + (((unsigned)(va) >> 20) & ~3))

/*
 * L1 entry for section containing va
 */
#define	KTOL1SC(va)				(L1_table + ((unsigned)(va) >> 20))
#define	UTOL1SC(va)				((ptp_t *)ARM_V6_USER_L1 + ((unsigned)(va) >> 20))

/*
 * L1 and L2 indexes for addresses in inactive address spaces
 */
#define	ITOL1PT(va)				((ptp_t *)ARM_V6_INACTIVE_L1 + (((unsigned)(va) >> 20) & ~3))
#define	ITOL1SC(va)				((ptp_t *)ARM_V6_INACTIVE_L1 + ((unsigned)(va) >> 20))
#define	ITOPTEP(va)				((pte_t *)ARM_V6_INACTIVE_L2 + (((unsigned)(va) >> 12) & 0x3ff))
#define	ITOPDIR(va)				((pte_t *)ARM_V6_SCRATCH_PTBL + ((unsigned)(va) >> 22))

/*
 * ARMv6 L2 descriptors use APX bit to determine writability.
 */
#define	ARM_PTE_WRITABLE(pte)	(((pte) & ARM_PTE_V6_APX) == 0)

/*
 * PTE_OP_BAD uses invalid pte with ARM_PTE_APX set
 */
#define	ARM_PTE_BAD				(ARM_PTE_V6_APX)

extern void			ptzero(struct pa_quantum *);
extern unsigned		l2_prot;

extern unsigned		v6_cache_colours(void);
extern void			xfer_init(void);

#define VX_CACHE_COLOURS()		v6_cache_colours()
#define VX_XFER_INIT()			xfer_init()

/* __SRCVERSION("vx_mm_internal.h $Rev: 173587 $"); */
