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
 *  mips/vm.h
 *
 */

#ifndef __MIPS_VM_H_INCLUDED
#define __MIPS_VM_H_INCLUDED

/*
 * Virtual address range available to all users
 * for read-only access to the system page. The
 * kernel can use this range as well, and it will
 * use KSEG0 addresses for writing to the page.
 * MIPS_USYSPAGES pages will be reserved at 
 * MIPS_USYSPAGES_BASE, and MIPS_USYSPAGE_PTR 
 * reflects the starting address where the pages 
 * are actually mapped. MIPS_UCPUPAGE_PTR is used
 * as the user virtual address for the CPU pages on
 * an SMP system.
 */
#define MIPS_USYSPAGES	4
#ifndef MIPS_USYSPAGES_BASE
#define MIPS_USYSPAGES_BASE	((0x8000000 + 0x20000) - (MIPS_USYSPAGES * __PAGESIZE))
#endif
#define MIPS_USYSPAGE_PTR	(MIPS_USYSPAGES_BASE + (2 * __PAGESIZE))
#define MIPS_UCPUPAGE_PTR	(MIPS_USYSPAGES_BASE + (0 * __PAGESIZE))

/* defines for physical pages */
#define PG_PFNMASK 		0xFFFFF000
#define PG_PFNSHIFT 	12

/*
 * Definitions for pte manipulations in tlb code. The first
 * set are pertinent to servicing tlb misses when CP0_BADVADDR
 * is referred to for the missing address. 
 */
#define PT_L1SHIFT              21
#define PT_L1MASK               0x7FF
#define PT_L1INDXSHIFT          2
#define PT_L2SHIFT              12
#define PT_L2MASK               0x1FF
#define PT_L2INDXSHIFT          3

/*
 * For picking apart the bits which index each level
 */
#define L1IDX(vaddr) (((uint32_t)vaddr) >> PT_L1SHIFT)
#define L2IDX(vaddr) ((((uint32_t)vaddr) >> PT_L2SHIFT) & PT_L2MASK)

#endif

/* __SRCVERSION("vm.h $Rev: 154853 $"); */
