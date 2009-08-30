/*
 * $QNXLicenseC: 
 * Copyright 2007, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */



/*
 * ARM MMU (coprocessor 15) Support
 */

#ifndef	__ARM_MMU_H_INCLUDED
#define	__ARM_MMU_H_INCLUDED

/*
 * MMU control register
 */
#define	ARM_MMU_CR_M	(1 <<  0)	/* enable MMU */
#define	ARM_MMU_CR_A	(1 <<  1)	/* enable alignment faults */
#define	ARM_MMU_CR_C	(1 <<  2)	/* enable data/IDC cache */
#define	ARM_MMU_CR_W	(1 <<  3)	/* enable write buffer */
#define	ARM_MMU_CR_P	(1 <<  4)	/* 32-bit exception handling */
#define	ARM_MMU_CR_D	(1 <<  5)	/* 32-bit data address range */
#define	ARM_MMU_CR_L	(1 <<  6)	/* late aborts */
#define	ARM_MMU_CR_B	(1 <<  7)	/* big endian */
#define	ARM_MMU_CR_S	(1 <<  8)	/* system protection */
#define	ARM_MMU_CR_R	(1 <<  9)	/* ROM protection */
#define	ARM_MMU_CR_F	(1 << 10)	/* implementation defined */
#define	ARM_MMU_CR_Z	(1 << 11)	/* implementation defined */
#define	ARM_MMU_CR_I	(1 << 12)	/* enable instruction cache */
#define	ARM_MMU_CR_X	(1 << 13)	/* exception vector adjust */
#define	ARM_MMU_CR_V	ARM_MMU_CR_X
#define	ARM_MMU_CR_RR	(1 << 14)	/* round robin cache replacement */
#define	ARM_MMU_CR_L4	(1 << 15)	/* load PC instr can set T bit */
#define	ARM_MMU_CR_DT	(1 << 16)	/* data TCM enable */
/* reserved				(1 << 17) */
#define	ARM_MMU_CR_IT	(1 << 18)	/* instruction TCM enable */
/* reserved				(1 << 19) */
/* reserved				(1 << 20) */
#define	ARM_MMU_CR_FI	(1 << 21)	/* fast interrupt config enable */
#define	ARM_MMU_CR_U	(1 << 22)	/* unaligned/mixed-endian enable */
#define	ARM_MMU_CR_XP	(1 << 23)	/* enable ARMv6 page table descriptors */
#define	ARM_MMU_CR_VE	(1 << 24)	/* vectored interrupt enable */
#define	ARM_MMU_CR_EE	(1 << 25)	/* mixed-endian exception entry */
#define	ARM_MMU_CR_L2	(1 << 26)	/* L2 unified cache enable */
#define	ARM_MMU_CR_NMFI	(1 << 27)	/* FIQs behave as NMFI */
#define	ARM_MMU_CR_TR	(1 << 28)	/* use TEX remap registers */
#define	ARM_MMU_CR_FA	(1 << 29)	/* force use of AP[0] as access bit */
#define	ARM_MMU_CR_nF	(1 << 30)	/* ARM920 only: not FastBus select */
#define	ARM_MMU_CR_iA	(1 << 31)	/* ARM920 only: async clock select */

#define	ARM_L1_SIZE		16384
#define	ARM_L2_SIZE		1024
#define	PGMASK			(__PAGESIZE-1)

#define	ARM_SCSIZE		(1024*1024)
#define	ARM_SCMASK		(ARM_SCSIZE-1)

#define	ARM_LPSIZE		65536
#define	ARM_LPMASK		(ARM_LPSIZE-1)

/*
 * Page/Section access permissions.
 * NB: assumes the MMU control register has been set with S=1,R=0
 */
#define	ARM_PTE_RO		0			/* read-only  access */
#define	ARM_PTE_RW		1			/* read-write access */
#define	ARM_PTE_U		2			/* user mode  access */
#define	ARM_PTE_AP_MASK	3

#define	ARM_PTE_B		4			/* bufferable */
#define	ARM_PTE_C		8			/* machine-dependent (see below) */
#define	ARM_PTE_CB		12			/* cacheable and bufferable */

/*
 * L1 descriptors
 */
typedef unsigned	ptp_t;
#define	ARM_PTP_L2		0x11
#define	ARM_PTP_SC		0x12
#define	ARM_PTP_V6_L2	0x01		/* ARMv6 L1 descriptor */
#define	ARM_PTP_V6_SC	0x02		/* ARMv6 L1 descriptor */
#define	ARM_PTP_VALID	3

#define	ARM_PTP_P		(1 << 9)	/* ECC enabled */
#define	ARM_PTP_SUPER	(1 <<18)	/* supersection descriptor */

/*
 * L1 section cacheability
 */
#define	ARM_PTP_B		4			/* bufferable */
#define	ARM_PTP_C		8			/* machine-dependent (see below) */
#define	ARM_PTP_CB		12			/* cacheable and bufferable */
#define	ARM_PTP_WB		ARM_PTP_CB	/* write-back cacheable */
#define	ARM_PTP_WT		ARM_PTP_C	/* write-thru cacheable */


/*
 * ARMv5 TEX bits
 */
#define	ARM_PTP_V5_TEX(x)	(((x) & 0xf) << 12)
#define	ARM_PTP_V5_TEX_MASK	ARM_PTP_V5_TEX(0xf)
#define	ARM_PTP_XS_WA		(ARM_PTP_CB | ARM_PTP_V5_TEX(1))	/* Xscale write-alloc */

/*
 * ARMv6 L1 descriptor bits
 */
#define	ARM_PTP_V6_XN		(1 << 4)
#define	ARM_PTP_V6_AP0		(1 << 10)
#define	ARM_PTP_V6_AP1		(1 << 11)
#define	ARM_PTP_V6_APX		(1 << 15)
#define	ARM_PTP_V6_S		(1 << 16)
#define	ARM_PTP_V6_nG		(1 << 17)

#define	ARM_PTP_V6_TEX(x)	(((x) & 7) << 12)
#define	ARM_PTP_V6_TEX_MASK	ARM_PTP_V6_TEX(7)

/*
 * Section access permissions when S=1,R=0
 */
#define	ARM_PTP_AP_RO		(0)			/* RO section permission */
#define	ARM_PTP_AP_RW		(1 << 10)	/* RW section permission */
#define	ARM_PTP_AP_U		(1 << 11)	/* user access allowed   */

/*
 * ARMv6 section access permissions
 *
 * APX bit determines whether write access is allowed
 * AP1 bit determines whether user access is allowed
 *
 * The encodings below reflect the ARMv6 encoding in ARMARMrevF:
 *
 * APX AP1 AP0
 * --- --- ---
 *   1   0   1	priv RO / user NA
 *   0   0   1	priv RW / user NA
 *   1   1   0	priv RO / user RO
 *   0   1   1	priv RW / user RW 
 */
#define	ARM_PTP_V6_AP_U		ARM_PTP_V6_AP1
#define	ARM_PTP_V6_AP_URO	(ARM_PTP_V6_AP1|ARM_PTP_V6_APX)
#define	ARM_PTP_V6_AP_URW	(ARM_PTP_V6_AP1|ARM_PTP_V6_AP0)
#define	ARM_PTP_V6_AP_KRO	(ARM_PTP_V6_AP0|ARM_PTP_V6_APX)
#define	ARM_PTP_V6_AP_KRW	(ARM_PTP_V6_AP0)

/*
 * L2 descriptors
 */
typedef unsigned	pte_t;
#define	ARM_PTE_LP		1
#define	ARM_PTE_SP		2
#define	ARM_PTE_XSP		3			/* ARMv5 extended small page */
#define	ARM_PTE_VALID	3

#define	ARM_PTE_PROT(f)	(((f) << 4) | ((f) << 6) | ((f) << 8) | ((f) << 10))
#define	ARM_XSP_PROT(f)	((f) << 4)

#define	ARM_PTE_WT		ARM_PTE_C			/* write-thru (non-SA11x0) */
#define	ARM_PTE_WB		ARM_PTE_CB			/* write-back (all cores)  */
#define	ARM_PTE_SA_MC	ARM_PTE_C			/* mini-cache (SA11x0)     */

#define	ARM_PTE_XS_X	0x40						/* Xscale X-bit */
#define	ARM_PTE_XS_MC	(ARM_PTE_XS_X | ARM_PTE_C)	/* mini-cache (Xscale) */
#define	ARM_PTE_XS_WA	(ARM_PTE_XS_X | ARM_PTE_WB)	/* write-allocate */

#define	ARM_PTE_V5_LP_TEX_MASK	0xf000
#define	ARM_PTE_V5_SP_TEX_MASK	0x3c0

#define	ARM_PTE_V6_AP0		(1 << 4)
#define	ARM_PTE_V6_AP1		(1 << 5)
#define	ARM_PTE_V6_APX		(1 << 9)
#define	ARM_PTE_V6_S		(1 << 10)
#define	ARM_PTE_V6_nG		(1 << 11)

/*
 * APX bit determines whether write access is allowed
 * AP1 bit determines whether user access is allowed
 *
 * The encodings below reflect the ARMv6 encoding in ARMARMrevF:
 *
 * APX AP1 AP0
 * --- --- ---
 *   1   0   1	priv RO / user NA
 *   0   0   1	priv RW / user NA
 *   1   1   0	priv RO / user RO
 *   0   1   1	priv RW / user RW 
 */
#define	ARM_PTE_V6_AP_U		ARM_PTE_V6_AP1
#define	ARM_PTE_V6_AP_URO	(ARM_PTE_V6_AP1|ARM_PTE_V6_APX)
#define	ARM_PTE_V6_AP_URW	(ARM_PTE_V6_AP1|ARM_PTE_V6_AP0)
#define	ARM_PTE_V6_AP_KRO	(ARM_PTE_V6_AP0|ARM_PTE_V6_APX)
#define	ARM_PTE_V6_AP_KRW	(ARM_PTE_V6_AP0)

#define	ARM_PTE_V6_SP_XN		(1 << 0)	/* XN for small pages only */
#define	ARM_PTE_V6_SP_TEX(x)	(((x) & 7) << 6)
#define	ARM_PTE_V6_SP_TEX_MASK	ARM_PTE_V6_SP_TEX(7)

#define	ARM_PTE_V6_LP_XN		(1 << 15)
#define	ARM_PTE_V6_LP_TEX(x)	(((x) & 7) << 12)
#define	ARM_PTE_V6_LP_TEX_MASK	ARM_PTE_V6_LP_TEX(7)

/*
 * ARMv6 TTBR0/TTBR1 attributes
 */
#define	ARM_MMU_TTBR_C		(1 << 0)			/* inner cacheable */
#define	ARM_MMU_TTBR_S		(1 << 1)			/* shared memory */
#define	ARM_MMU_TTBR_P		(1 << 2)			/* ECC enabled */
#define	ARM_MMU_TTBR_RGN(x)	(((x) & 3) << 3)	/* outer cacheable type */

/*
 * Address translation support.
 *
 * The virtual address space contains the following reserved regions:
 *	ff800000-ffbfffff	maps page tables that map 00000000-ffffffff
 *	ffff0000-ffff0fff	trap vector table for processors with vector adjust
 *
 * The startup normally arranges the following mappings:
 *	e0000000-efffffff	maps up to 256MB of system RAM
 *	fc400000-fdffffff	maps L1 page table and mappings for callouts
 *	fe000000-ff7fffff	maps boot programs (kdebug/procnto)
 *
 * For ARMv6 processors, there are some additions:
 *
 *	fc000000-fc1fffff	maps page tables for user address space on ARMv6
 *	fc200000-fc3fffff	reserved (mapped by user "page directory" page
 *	ff000000-ff7fffff	used for message passing
 *	ffc00000-ffc3ffff	used for cache colour operations
 *	ffff1000-ffff1fff	cpupage on SMP systems
 *	ffff2000-ffff3fff	8K L1 table for ARMv6 user address space
 *	ffff4000-ffff7fff	8K used to map L1 tables for message passing
 *	ffff8000-ffff9fff	8K used to map L1 tables for inactive address spaces
 *	ffffa000-ffffafff	4K used to map L2 tables for inactive address spaces
 *	ffffe000-ffffefff	scratch page used for zeroing page tables
 *	fffff000-ffffffff	scratch page used for planting breakpoints
 */

#define	ARM_1TO1_BASE		0xe0000000
#define	ARM_1TO1_SIZE		0x10000000

#define	ARM_STARTUP_BASE	0xfc400000
#define	ARM_BOOTPGM_BASE	0xfe000000

#define	ARM_PTE_MAP			1022
#define	ARM_PTE_BASE		(ARM_PTE_MAP << 22)
#define	ARM_PTP_BASE		(ARM_PTE_BASE | (ARM_PTE_MAP << 12))

#define	VTOPDIR(v)	(ptp_t *)(ARM_PTP_BASE | (((unsigned)(v) >> 20) & ~3))
#define	VTOPTEP(v)	(pte_t *)(ARM_PTE_BASE | (((unsigned)(v) >> 10) & ~3))
#define	VTOPTP(v)	(pte_t *)(ARM_PTE_BASE | (((unsigned)(v) >> 10) & 0x3ff000))
#define	VTOP(v)		((*(VTOPTEP(v)) & ~PG_MASK) | ((unsigned)(v) & PG_MASK))

/*
 * ARMv6-specific macros
 */
#define	ARM_V6_XFER_BASE	0xff000000
#define	ARM_V6_XFER_SIZE	0x00800000
#define	ARM_SMP_CPUPAGE		0xffff1000
#define	ARM_V6_USER_L1		0xffff2000
#define	ARM_V6_XFER_L1		0xffff4000
#define	ARM_V6_INACTIVE_L1	0xffff8000
#define	ARM_V6_INACTIVE_L2	0xffffa000
#define	ARM_V6_SCRATCH_PTBL	0xffffe000
#define	ARM_V6_SCRATCH_BKPT	0xfffff000
#define	ARM_V6_COLOUR_BASE	0xffc00000

/*
 * Macros to manipulate system mappings (80000000-ffffffff)
 */
#define	ARM_KPTE_BASE	ARM_PTE_BASE
#define	ARM_KPTP_BASE	ARM_PTP_BASE
#define	KTOPDIR(v)		VTOPDIR(v)
#define	KTOPTEP(v)		VTOPTEP(v)
#define	KTOPTP(v)		VTOPTP(v)
#define	KTOP(v)			VTOP(v)

/*
 * Macros to manipulate user mappings (00000000-7fffffff)
 */
#define	ARM_UPTE_MAP	1008
#define	ARM_UPTE_BASE	(ARM_UPTE_MAP << 22)
#define	ARM_UPTP_BASE	(ARM_KPTE_BASE | (ARM_UPTE_MAP << 12))
#define	UTOPDIR(v)		(ptp_t *)(ARM_UPTP_BASE | (((unsigned)(v) >> 20) & ~3))
#define	UTOPTEP(v)		(pte_t *)(ARM_UPTE_BASE | (((unsigned)(v) >> 10) & ~3))
#define	UTOPTP(v)		(pte_t *)(ARM_UPTE_BASE | (((unsigned)(v) >> 10) & 0x3ff000))
#define	UTOP(v)			((*(UTOPTEP(v)) & ~PG_MASK) | ((unsigned)(v) & PG_MASK))

/*
 * --------------------------------------------------------------------------
 * Cache/TLB manipulation
 * --------------------------------------------------------------------------
 */

/*
 * Read MMU ID register
 */
static __inline__ unsigned
arm_mmu_cpuid()
{
	unsigned	val;
	__asm__ __volatile__("mrc	p15, 0, %0, c0, c0, 0" : "=r" (val));
	return val;
}

/*
 * Read MMU cache type register
 */
static __inline__ unsigned
arm_mmu_cache_type()
{
	unsigned	val;
	__asm__ __volatile__("mrc	p15, 0, %0, c0, c0, 1" : "=r" (val));
	return val;
}

/*
 * Read the MMU control register
 */
static __inline__ unsigned
arm_mmu_getcr()
{
	unsigned	val;
	__asm__ __volatile__("mrc	p15, 0, %0, c1, c0, 0" : "=r" (val));
	return val;
}

/*
 * Flush data TLBs
 */
static __inline__ void
arm_v4_dtlb_flush()
{
	__asm__ __volatile__("mcr	p15, 0, %0, c8, c6, 0" : : "r" (0));
}

/*
 * Flush the instruction and data TLBs
 */
static __inline__ void
arm_v4_idtlb_flush()
{
	__asm__ __volatile__("mcr	p15, 0, %0, c8, c7, 0" : : "r" (0));
}

/*
 * Flush the data TLB by address
 */
static __inline__ void
arm_v4_dtlb_addr(unsigned addr)
{
	__asm__ __volatile__("mcr	p15, 0, %0, c8, c6, 1" : : "r" (addr));
}

/*
 * Flush the instruction and data TLBs by address
 */
static __inline__ void
arm_v4_idtlb_addr(unsigned addr)
{
	__asm__ __volatile__("mcr	p15, 0, %0, c8, c7, 1" : : "r" (addr));
}

/*
 * Get FCSE PID register
 */
static __inline__ unsigned
arm_v4_fcse_get()
{
	unsigned	pid;
	__asm__ __volatile__("mrc	p15, 0, %0, c13, c0, 0" : "=r" (pid));
	return pid;
}
 
/*
 * Set FCSE PID register
 */
static __inline__ void
arm_v4_fcse_set(unsigned pid)
{
	__asm__ __volatile__("mcr	p15, 0, %0, c13, c0, 0" : : "r" (pid));
}

/*
 * Data Synchronisation barrier
 */
static __inline__ void
arm_v6_dsb()
{
	__asm__ __volatile__("mcr	p15, 0, %0, c7, c10, 4" : : "r" (0));
}

/*
 * Data Memory Barrier
 */
static __inline__ void
arm_v6_dmb()
{
	__asm__ __volatile__("mcr	p15, 0, %0, c7, c10, 5" : : "r" (0));
}

/*
 * Set TTBR0 register
 */
static __inline__ void
arm_v6_ttbr0_set(unsigned ttbr0)
{
	__asm__ __volatile__("mcr	p15, 0, %0, c2, c0, 0" : : "r" (ttbr0));
}

/*
 * Get TTBR0 register
 */
static __inline__ unsigned
arm_v6_ttbr0_get()
{
	unsigned	ttbr0;

	__asm__ __volatile__("mrc	p15, 0, %0, c2, c0, 0" :  "=r" (ttbr0));
	return ttbr0;
}

/*
 * Set current ASID
 */
static __inline__ void
arm_v6_asid_set(unsigned asid)
{
	__asm__ __volatile__("mcr	p15, 0, %0, c13, c0, 1" : : "r" (asid));
}

/*
 * Flush Branch Target Cache
 */
static __inline__ void
arm_v6_flush_btc()
{
	__asm__ __volatile__("mcr	p15, 0, %0, c7, c5, 6" : : "r" (0));
}

/*
 * Flush all TLB entries matching ASID
 */
static __inline__ void
arm_v6_tlb_asid(unsigned asid)
{
	__asm__ __volatile__("mcr	p15, 0, %0, c8, c7, 2" : : "r" (asid));
}

/*
 * Get CPU number on MPcore
 */
static __inline__ unsigned
arm_v6_cpunum()
{
	unsigned	cpunum;
	__asm__ __volatile__("mrc	p15, 0, %0, c0, c0, 5" : "=r" (cpunum));
	return cpunum;
}

#endif	/* __ARM_MMU_H_INCLUDED */

/* __SRCVERSION("mmu.h $Rev: 160127 $"); */
