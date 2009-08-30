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

unsigned
get_spr_indirect(unsigned spr) {
	unsigned	instrs[2];

	spr = (spr >> 5) | ((spr & 0x1f) << 5);
	instrs[0] = 0x7c6002a6 | (spr << 11); 	// mfspr %r3,SPR
	instrs[1] = 0x4e800020;					// blr

	// flush both in case we're crossing a cache boundry.
	icache_flush(&instrs[0]);
	icache_flush(&instrs[1]);

	return ((unsigned (*)(void))instrs)();
}

void
set_spr_indirect(unsigned spr, unsigned value) {
	unsigned	instrs[2];

	spr = (spr >> 5) | ((spr & 0x1f) << 5);
	instrs[0] = 0x7c6003a6 | (spr << 11); 	// mtspr SPR,%r3
	instrs[1] = 0x4e800020;					// blr

	// flush both in case we're crossing a cache boundry.
	icache_flush(&instrs[0]);
	icache_flush(&instrs[1]);

	((void (*)(unsigned))instrs)(value);
}

__SRCVERSION("spr_indirect.c $Rev: 153052 $");
