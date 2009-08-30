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
 *  ppc/syspage.h
 *

 */

#ifndef __SYSPAGE_H_INCLUDED
#error ppc/syspage.h should not be included directly.
#endif

#ifdef __PPC__

extern struct cpupage_entry *_cpupage_ptr;

#endif /* __PPC__ */
 
/*
 *	CPU capability/state flags
 */
#define PPC_CPU_EAR			(1UL <<  0)  /* CPU has EAR register */
#define PPC_CPU_HW_HT		(1UL <<  1)  /* CPU uses HW hash table */
#define PPC_CPU_HW_POW		(1UL <<  2)  /* CPU has power management */
#define PPC_CPU_FPREGS		(1UL <<  3)  /* CPU has floating point registers */
#define PPC_CPU_SW_HT		(1UL <<  4)  /* CPU uses a SW hash table */
#define PPC_CPU_ALTIVEC		(1UL <<  5)  /* CPU supports Altivec */
#define PPC_CPU_XAEN		(1UL <<  6)  /* CPU supports extended addressing */
#define PPC_CPU_SW_TLBSYNC	(1UL <<  7)  /* CPU needs SW to sync TLB's */
#define PPC_CPU_TLB_SHADOW	(1UL <<  8)  /* CPU has shadow regs in TLB handler */
#define PPC_CPU_DCBZ_NONCOHERENT (1UL <<  9)  /* DCBZ has problems with multiple CPU's */
#define PPC_CPU_SPE 		(1UL <<  10) /* CPU supports SPE */
#define PPC_CPU_NO_MFTB		(1UL <<  11) /* CPU doesn't support MFTB[U] */
#define PPC_CPU_EXTRA_BAT	(1UL <<  12) /* CPU has extra BAT registers */
#define PPC_CPU_STWCX_BUG	(1UL <<  13) /* CPU has an errata with STWCX. */

/*
 * intrinfo_entry flags
 */
#define PPC_INTR_FLAG_400ALT	0x0001U
#define PPC_INTR_FLAG_CI		0x0001U
#define PPC_INTR_FLAG_SHORTVEC	0x0002U

#if defined(ENABLE_DEPRECATED_SYSPAGE_SECTIONS)
/*
 *	Support hardware flags
 */
#define PPC_HW_BUS_ISA		(1UL <<  0)	/* Machine has ISA bus */
#define PPC_HW_BUS_EISA		(1UL <<  1)	/* Machine has EISA bus */
#define PPC_HW_BUS_PCI		(1UL <<  2)  /* Machine has PCI bus */
#define PPC_HW_BUS_MCA		(1UL <<  3)	/* Machine has micro channel bus */

struct ppc_boxinfo_entry {
	unsigned long	hw_flags;			/* PPC_HW_* */
	unsigned long	spare [9];
};
#endif

enum {
	PPC_FAMILY_UNKNOWN,
	PPC_FAMILY_400,
	PPC_FAMILY_600,
	PPC_FAMILY_800,
	PPC_FAMILY_booke,
	PPC_FAMILY_900
};

struct ppc_kerinfo_entry {
	unsigned long	pretend_cpu;		/* can pretend chip is this PVR */
	unsigned long	init_msr;			/* initial MSR for thread creation */
	unsigned long	ppc_family;
	unsigned long   asid_bits;
	unsigned long	callout_ts_clear;
	unsigned long   spare[6];
};

struct ppc_smpinfo_entry {
	_SPPTR(void)		mpu_start_addr;
	int					ipi_vector;
	_SPFPTR 			(void, send_ipi, (unsigned, int, unsigned *));			
	unsigned long		spare[9];
};

/* For Book E only */
struct ppc_tlbinfo_entry {
	unsigned short		num_entries;
	unsigned short		page_sizes;
	unsigned short		spare[2];
};

struct ppc_syspage_entry {
	syspage_entry_info	DEPRECATED_SECTION_NAME(boxinfo);
	syspage_entry_info	kerinfo;
	_SPPTR(_Uint32t)	exceptptr;
	syspage_entry_info	DEPRECATED_SECTION_NAME(smpinfo);
	syspage_entry_info	tlbinfo;
};

struct ppc_kernel_entry {
	unsigned long	code[2];
};

/* __SRCVERSION("syspage.h $Rev: 159574 $"); */
