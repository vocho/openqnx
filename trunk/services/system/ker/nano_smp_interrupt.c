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

//
// Syncronize all the controller mask states on an SMP system that need
// it. A controller (e.g. MIPS) that has per CPU registers is expected
// to keep a global state in memory. A mask (or unmask) callout with
// a -1 vector number is a request to synchronize with that global
// state.
//
void rdecl
interrupt_smp_sync(unsigned flag) {
	struct intrinfo_entry	*iip;
	unsigned				i;

	CPU_SLOCK_LOCK(&intr_slock);
	iip = intrinfoptr;
	for(i = 0; i < intrinfo_num; ++i, ++iip) {
		if(iip->flags & flag) {
			(void)iip->mask(_syspage_ptr, -1);
		}
	}
	CPU_SLOCK_UNLOCK(&intr_slock);
}

__SRCVERSION("nano_smp_interrupt.c $Rev: 153052 $");
