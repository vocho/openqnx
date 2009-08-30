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
 *  mips/syspage.h
 *

 */

#ifndef __SYSPAGE_H_INCLUDED
#error mips/syspage.h should not be included directly.
#endif

#ifdef __MIPS__

extern struct cpupage_entry *_cpupage_ptr;

#endif /* __MIPS__ */
 
/*
 *	CPU capability/state flags
 */
#define MIPS_CPU_FLAG_PFNTOPSHIFT_MASK	0x000f /* used for constructing TLB entries */
#define MIPS_CPU_FLAG_MAX_PGSIZE_MASK	0x00f0
#define MIPS_CPU_FLAG_MAX_PGSIZE_SHIFT	0x0003 /* 3 _is_ the correct number */
#define MIPS_CPU_FLAG_L2_PAGE_CACHE_OPS	0x0100
#define MIPS_CPU_FLAG_64BIT				0x0200 /* has  64 bit registers */
#define MIPS_CPU_FLAG_128BIT			0x0400 /* has 128 bit registers */
#define MIPS_CPU_FLAG_SUPERVISOR		0x0800 /* has supervisor mode */
#define MIPS_CPU_FLAG_NO_WIRED			0x1000 /* doesn't have wired reg */
#define MIPS_CPU_FLAG_NO_COUNT			0x2000 /* doesn't have count reg */

#if defined(ENABLE_DEPRECATED_SYSPAGE_SECTIONS)
/*
 *	Support hardware flags
 */
#define MIPS_HW_BUS_ISA   (1 <<  0)	/* Machine has ISA bus */
#define MIPS_HW_BUS_EISA  (1 <<  1)	/* Machine has EISA bus */
#define MIPS_HW_BUS_PCI   (1 <<  2)	/* Machine has PCI bus */
#define MIPS_HW_BUS_MCA	  (1 <<  3)	/* Machine has micro channel bus */

struct mips_boxinfo_entry {
	unsigned long	hw_flags;	/* hardware flags */
	unsigned long	spare[9];
};
#endif

struct mips_ptehack_entry {
	unsigned long		prot_mask;
	unsigned long		prot_check;
	unsigned long		pte_clr;
	unsigned long		pte_set;
};

struct mips_syspage_entry {
	syspage_entry_info		DEPRECATED_SECTION_NAME(boxinfo);
	syspage_entry_info		ptehack;
	volatile unsigned long	shadow_imask;
};

struct mips_kernel_entry {
	_Uint32t 				code[2];
};

/* __SRCVERSION("syspage.h $Rev: 153052 $"); */
