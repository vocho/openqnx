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
 *  arm/syspage.h
 *

 */

#ifndef __SYSPAGE_H_INCLUDED
#error arm/syspage.h should not be included directly.
#endif

#ifdef __ARM__

extern struct cpupage_entry *_cpupage_ptr;

#endif /* __ARM__ */

/*
 *	CPU capability/state flags
 */
#define	ARM_CPU_FLAG_XSCALE_CP0		0x0001		/* Xscale CP0 MAC unit */
#define	ARM_CPU_FLAG_V6				0x0002		/* ARMv6 cpu */
#define	ARM_CPU_FLAG_V6_ASID		0x0004		/* use ARMv6 MMU ASID */

#if defined(ENABLE_DEPRECATED_SYSPAGE_SECTIONS)
struct	arm_boxinfo_entry {
	_Uint32t		hw_flags;			/* HW_* */
	_Uint32t		spare[9];
};
#endif

struct	arm_cpu_entry {
	_SPFPTR(int,	page_flush,				(unsigned, int));
	_SPFPTR(void,	page_flush_deferred,	(int));
	_Uint16t	upte_ro;
	_Uint16t	upte_rw;
	_Uint16t	kpte_ro;
	_Uint16t	kpte_rw;
	_Uint16t	mask_nc;
	_Uint16t	spare0;
	_Uint32t	spare[3];
};

struct arm_syspage_entry {
	syspage_entry_info	DEPRECATED_SECTION_NAME(boxinfo);
	_Uint32t	 		L1_vaddr;
#if defined(__GNUC__) 	
	/* Old startups referenced L1_paddr (even though they didn't
	   have to), so let's do some hackery to let them continue to compile
	*/   
	union {
		_Uint32t		L2_vaddr;
		_Uint32t		L1_paddr;
	}; 
#else	
	_Uint32t			L2_vaddr;
#endif	
	_Uint32t			startup_base;
	unsigned			startup_size;
	syspage_entry_info	cpu;
};

struct arm_kernel_entry {
	unsigned long	code[2];
};

/* __SRCVERSION("syspage.h $Rev: 160127 $"); */
