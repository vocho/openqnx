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


int rdecl
within_syspage(uintptr_t vaddr, unsigned size) {
	uintptr_t	end   = vaddr + size - 1;

	if(end <= vaddr) return 0;
	if(vaddr < (uintptr_t)privateptr->user_syspageptr) return 0;
	if(end > ((uintptr_t)privateptr->user_syspageptr + _syspage_ptr->total_size - 1)) return 0;
	return 1;
}

__SRCVERSION("nano_syspage.c $Rev: 153052 $");
