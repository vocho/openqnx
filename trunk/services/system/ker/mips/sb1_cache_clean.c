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

#include "externs.h"
#include <mips/vm.h>

__SRCVERSION("sb1_cache_clean.c $Rev: 153052 $");

/* 
 * Broadcom SB-1 Specific method for cleaning out corrupt cache data when the
 * tag is stil known to be good.
 */

// create a mask that isolates bits 39:13
#define PA_MASK	((((paddr_t)1 << 40) - 1) ^ ((1 << 14) - 1))

void 
sb1_cache_clean(paddr_t	bad_paddr) {
	uint64_t	taglo=0;
	int			way;
	uintptr_t	cache_vaddr;

	/* we're going to check each way, so remove the way bits 14:13 from 
	   the address */
	cache_vaddr = (uintptr_t)bad_paddr & ~0x6000;
	bad_paddr &= PA_MASK;

	way = 3;
	do {

		/*
		 * get TAGLO
		 */
		asm volatile (
			".set mips64;"
			".set noreorder;"
			".set noat;"
			"cache 	5,0(%0);" /* index ld tag data */
			" nop;"
			"dmfc0	$1,$28,2;"
			" nop;"
			"sd		$1,0(%1);"
			".set reorder;"
			".set mips2;"
			: /* no outputs */
			: "r" (cache_vaddr), "r"(&taglo)
			: "memory"); 
					
		if((taglo & PA_MASK) == bad_paddr) {

			/*
			 * invalidate it by clearing TAGHI
			 */
			asm volatile (
				".set mips64;"
				".set noreorder;"
				"mtc0	$0,$29,2;"
				" nop;"
				"cache 	9,0(%0);" /* index ld tag data */
				" nop;"
				".set reorder;"
				".set mips2;"
				: /* no outputs */
				: "r" (cache_vaddr)); 
		}
		cache_vaddr += 0x2000; // advance to next way
	} while(--way >= 0);
}
