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
 *  sh/syspage.h
 *

 */

#ifndef __SYSPAGE_H_INCLUDED
#error sh/syspage.h should not be included directly.
#endif

#ifdef __SH__

extern struct cpupage_entry *_cpupage_ptr;

#endif /* __SH__ */
 
#if defined(ENABLE_DEPRECATED_SYSPAGE_SECTIONS)
/*
 *	Support hardware flags
 */
#define SH_HW_BUS_ISA		(1UL <<  0)	/* Machine has ISA bus */
#define SH_HW_BUS_EISA		(1UL <<  1)	/* Machine has EISA bus */
#define SH_HW_BUS_PCI		(1UL <<  2)	/* Machine has PCI bus */
#define SH_HW_BUS_MCA		(1UL <<  3)	/* Machine has micro channel bus */
#define SH_HW_BUS_VME		(1UL <<  2)	/* Machine has VME bus */

struct sh_boxinfo_entry {
	unsigned long	hw_flags;		
	unsigned long	spare [9];
};
#endif

struct sh_syspage_entry {
	syspage_entry_info	DEPRECATED_SECTION_NAME(boxinfo);
	_SPPTR(_Uint32t)	exceptptr;
	volatile unsigned long imask;
	unsigned long	flags;
};

#define SH_SYSPAGE_FLAGS_PERMANENT_MAP		0x1


#define	SH_CPU_FLAG_MOVLICO				(0x1 << 0)  	/* 0x0001 */
	/* Has movli/movco instructions */
#define	SH_CPU_FLAG_SMP					(0x1 << 1)	  	/* 0x0002 */
	/* SMP multi-cpu system */

#define SH_CPU_FLAG_IGNORE_OLD_PVR		(0x1 << 2)  	/* 0x0004 */
	/* Old startups didn't set the cpu flags correctly -- instead procnto
	 * looked at the PVR to differentiate behaviour on different CPUs.
	 * This meant that procnto potentially had to change every time a 
	 * new CPU with the new PVR value was introduced.
	 * 
	 * The correct solution is to have the startups set capability flags
	 * -- that's what these flags are all about -- but we still want to
	 * support older startups where the flags aren't set correctly.  
	 * 
	 * The IGNORE_OLD_PVR flag is used by newer startups to indicate that
	 * procnto doesn't need to consult the PVR, even if it's one that
	 * procnto knows about.  All new startups should set this flag and
	 * all other cpu flags appropriately.  If this flag is zero, as will
	 * be the case for old startups, it tells procnto that it should
	 * examine the PVR to determine CPU capabilitites.
	 * 
	 * All startups should set this flag.  It is only acceptable to have it
	 * clear for startups that were built prior to it being introduced.
     */
     
#define SH_CPU_FLAG_HAS_PCI				(0x1 << 3)  	/* 0x0008 */
	/* HAS_PCI - CPU has built-in PCI support.  Unused? */
	
#define SH_CPU_FLAG_USE_EMODE			(0x1 << 4)   	/* 0x0010 */
	/* USE_EMODE - turn on EMODE in CCR (7750r/7751r only) */
#define SH_CPU_FLAG_P2_FOR_CACHE		(0x1 << 5)  	/* 0x0020 */
	/* code manipulating the cache or TLB tables must run in P2 area */
#define SH_CPU_FLAG_TMU_THROUGH_A7		(0x1 << 6)  	/* 0x0040 */
	/* user-mode code looking at TMU must look through A7 area */
#define SH_CPU_FLAG_SHM_SPECIAL_BITS	(0x1 << 7)  	/* 0x0080 */
	/* allow use of special field in shm_ctl_special */
	
#define SH_CPU_FLAG_32_BIT				(0x1 << 8)  	/* 0x0100 */
	/* supports 32-bit physical address space (i.e. SH4a architecture */
	


struct sh_kernel_entry {
	unsigned long	code[2];
};

struct sh_cpupage_entry {
	volatile unsigned long	imask;
};

/* __SRCVERSION("syspage.h $Rev: 159814 $"); */
