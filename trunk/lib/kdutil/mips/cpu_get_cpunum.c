/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <mips/cpu.h>
#include "kdintl.h"


unsigned
cpu_get_cpunum(void) {
	unsigned	num;

	// CPU number is stored in the top 7 bits of the XCONTEXT register
	__asm__ __volatile__ (	
		".set mips3			;"
		".set noreorder		;"
		"dmfc0 	%0, $20 	;"
		"sll	$0,$0,1		;"
		"dsrl	%0,%0,64-7	;"
		".set reorder		;"
		".set mips2			;"
		: "=r" (num));

	return num;
}
